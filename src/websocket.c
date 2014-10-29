#include <stdio.h>
#include "client.h"
#include "websocket.h"
#include "base64.h"
#include "sha1.h"

char *
get_header_value(char *buffer, const char *key)
{
    char *b, *e, *value;
    
    b = strstr(buffer, key);
    if (b == NULL) 
        return NULL;

    b += strlen(key) + 2; // ': '
    
    e = strstr(b, "\r\n");
    if (e == NULL) 
        return NULL;

    value = malloc(e-b+1);
    bzero(value, 0);
    memcpy(value, b, e-b);

    return value;
}

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
    key = get_header_value(c->buffer, "Sec-WebSocket-Key");
    if (key == NULL) {
        return 0;
    }

    // Вычисляем хеш
    hash = get_handshake_hash(key);
    
    printf("KEY: |%s|\n", key);
    printf("HASH: |%s|\n", hash);

    // Формируем ответ
    answer = malloc(4096);
    bzero(answer, 4096);
    
    strcpy(answer, "HTTP/1.1 101 Switching Protocols\r\n");
    strcat(answer, "Upgrade: websocket\r\n");
    strcat(answer, "Connection: Upgrade\r\n");
    strcat(answer, "Sec-WebSocket-Accept: ");
    strcat(answer, hash);
    strcat(answer, "\r\n\r\n");
    
    // Отправляем ответ
    write(c->sock, answer, strlen(answer));
    printf(">>\n%s\n", answer);

    return 1;
}