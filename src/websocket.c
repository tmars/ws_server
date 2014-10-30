#include <stdio.h>
#include "client.h"
#include "websocket.h"
#include "base64.h"
#include "sha1.h"
#include "http.h"

#define ws_ntohl64(p) \
    ((((uint64_t)((p)[0])) <<  0) + (((uint64_t)((p)[1])) <<  8) +\
     (((uint64_t)((p)[2])) << 16) + (((uint64_t)((p)[3])) << 24) +\
     (((uint64_t)((p)[4])) << 32) + (((uint64_t)((p)[5])) << 40) +\
     (((uint64_t)((p)[6])) << 48) + (((uint64_t)((p)[7])) << 56))

#define ws_htonl64(p) {\
    (char)(((p & ((uint64_t)0xff <<  0)) >>  0) & 0xff), (char)(((p & ((uint64_t)0xff <<  8)) >>  8) & 0xff), \
    (char)(((p & ((uint64_t)0xff << 16)) >> 16) & 0xff), (char)(((p & ((uint64_t)0xff << 24)) >> 24) & 0xff), \
    (char)(((p & ((uint64_t)0xff << 32)) >> 32) & 0xff), (char)(((p & ((uint64_t)0xff << 40)) >> 40) & 0xff), \
    (char)(((p & ((uint64_t)0xff << 48)) >> 48) & 0xff), (char)(((p & ((uint64_t)0xff << 56)) >> 56) & 0xff) }

char *
get_handshake_hash(const char *key)
{
    unsigned char *buffer, sha1_output[20];
    char magic[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    
    // websocket handshake
    size_t key_sz = key?strlen(key):0, buffer_sz = key_sz + sizeof(magic) - 1;
    buffer = calloc(buffer_sz, 1);

    // concatenate key and guid in buffer
    memcpy(buffer, key, key_sz);
    memcpy(buffer+key_sz, magic, sizeof(magic)-1);

    // compute sha-1
    SHA1_CTX *ctx = calloc(1, sizeof(SHA1_CTX));

    SHA1_Init(ctx);
    SHA1_Update(ctx, buffer, buffer_sz);
    SHA1_Final(sha1_output, ctx);
    
    // encode `sha1_output' in base 64, into `out'.
    size_t l;
    return base64_encode(sha1_output, 20, &l);
}

int 
process_handshake(struct ws_client *c)
{
    char *key, *hash, *answer;
    int n;

    // Читаем данные
    n = ws_client_read(c);
    if (n <= 0) {
        return 0;
    }
    printf("<<\n%s\n", c->buffer);

    // Выбираем из заголовка ключ
    key = http_get_header_value(c->buffer, "Sec-WebSocket-Key");
    if (key == NULL) {
        return 0;
    }

    // Вычисляем хеш
    hash = get_handshake_hash(key);
    
    printf("KEY: |%s|\n", key);
    printf("HASH: |%s|\n", hash);


    // Формируем ответ
    struct http_response *r = http_response_init(101, "Switching Protocols");
    http_response_set_header(r, "Upgrade", "websocket");
    http_response_set_header(r, "Connection", "Upgrade");
    http_response_set_header(r, "Sec-WebSocket-Accept", hash);
    
    http_response_write(r);

    // Отправляем ответ
    write(c->sock, r->out, r->out_sz);
    printf(">>\n%s\n", r->out);

    http_response_free(r);
    free(key);
    free(hash);
    
    return 1;
}

char *
parse_data(const char *frame, int size)
{
    int has_mask;
    uint64_t len;
    const char *payload; // указатель на payload
    unsigned char mask[4];
    
    payload = frame;

    // Проверка полного фрейма
    /*if(size < 8) {
        return WS_READING;
    }*/

    // 8 бит
    has_mask = frame[1] & 0x80 ? 1:0;

    // Извлечение размера payload
    len = frame[1] & 0x7f;  // убираем 8 бит
    if (len <= 125) { 
        payload = frame + 2;
    } 
    else if(len == 126) {
        // Размер payload 16-ое число
        uint16_t size16;
        memcpy(&size16, frame + 2, sizeof(uint16_t));
        len = ntohs(size16);
        payload = frame + 4;
    } 
    else if(len == 127) {
        len = ws_ntohl64(frame + 2);
        payload = frame + 10;
    } 
    else {
        return NULL;
        //return WS_ERROR;
    }

    // данные начинаются сразу после макси
    if (has_mask) {
        memcpy(&mask, payload, sizeof(mask));
        payload += 4;
    }

    // Неостаточно данных
    if(len > size - (payload - frame)) {
        return NULL;
        //return WS_READING;
    }

    
    if(frame[0] & 0x80) { // FIN bit set
        char *data = malloc(len+1);
        memcpy(data, payload, len);
        data[len] = '\0';
        
        size_t i;
        for(i = 0; i < len && mask; ++i) {
            data[i] = (unsigned char)payload[i] ^ mask[i%4];
        }

        return data;

    } else {
        return NULL;
        //return WS_READING;
    }
}

char *
new_msg(const char *payload, int size, int *frame_size)
{
    char *frame = malloc(size + 8); // фрейм с учетом header
    *frame_size = 0;
    if (frame == NULL)
        return NULL;
    
    frame[0] = 0x81; // 10000001
    if (size <= 125) {
        frame[1] = size;
        memcpy(frame + 2, payload, size);
        *frame_size = size + 2;
    } 
    else if (size > 125 && size <= 65536) {
        uint16_t size16 = htons(size);
        frame[1] = 126;
        memcpy(frame + 2, &size16, 2);
        memcpy(frame + 4, payload, size);
        *frame_size = size + 4;
    } 
    else if (size > 65536) {
        char size64[8] = ws_htonl64(size);
        frame[1] = 127;
        memcpy(frame + 2, size64, 8);
        memcpy(frame + 10, payload, size);
        *frame_size = size + 10;
    }

    return frame;
}