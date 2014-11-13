#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "websocket.h"
#include "lib/base64.h"
#include "lib/sha1.h"

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

struct http_response *
get_handshake_response(char *header)
{
    char *key, *hash, *answer;
    int n;

    // Выбираем из заголовка ключ
    key = http_get_header_value(header, "Sec-WebSocket-Key");
    if (key == NULL) {
        return NULL;
    }

    // Вычисляем хеш
    hash = compute_handshake_hash(key);

    // printf("KEY: |%s|\n", key);
    // printf("HASH: |%s|\n", hash);

    // Формируем ответ
    struct http_response *response = http_response_init(101, "Switching Protocols");
    http_response_set_header(response, "Upgrade", "websocket");
    http_response_set_header(response, "Connection", "Upgrade");
    http_response_set_header(response, "Sec-WebSocket-Accept", hash);

    http_response_pack(response);

    free(key);
    free(hash);

    return response;
}
