#include <stdio.h>
#include "client.h"
#include "websocket.h"
#include "lib/base64.h"
#include "lib/sha1.h"
#include "http.h"

char *
compute_handshake_hash(const char *key)
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
    hash = compute_handshake_hash(key);

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
    printf(">>%d\n%s\n", r->out_sz, r->out);

    http_response_free(r);
    free(key);
    free(hash);

    return 1;
}
