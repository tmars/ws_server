#include "http.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct http_response *
http_response_init(int16_t code, const char *msg)
{
    struct http_response *r = calloc(1, sizeof(struct http_response));

    r->code = code;
    r->msg = msg;

    return r;
}

void
http_response_set_header(struct http_response *r, const char *k, const char *v)
{
    int i, pos = r->header_count;
    size_t key_size = strlen(k);
    size_t val_size = strlen(v);

    for (i = 0; i < r->header_count; ++i) {
        if (strncmp(r->headers[i].key, k, key_size) == 0) {
            pos = i;
            // убираем старое значение перед заменой
            free(r->headers[i].key);
            free(r->headers[i].val);
            break;
        }
    }

    // расширяем массив
    if (pos == r->header_count) {
        r->header_count++;
        r->headers = realloc(r->headers, sizeof(struct http_header)*r->header_count);
    }

    // копируем ключ
    r->headers[pos].key = calloc(key_size + 1, 1);
    memcpy(r->headers[pos].key, k, key_size);
    r->headers[pos].key_size = key_size;

    // копируем значение
    r->headers[pos].val = calloc(val_size + 1, 1);
    memcpy(r->headers[pos].val, v, val_size);
    r->headers[pos].val_size = val_size;
}

void
http_response_pack(struct http_response *r)
{
    char *p;
    int i, ret;

    r->out_size = sizeof("HTTP/1.1 xxx ")-1 + strlen(r->msg) + 2;
    r->out = calloc(r->out_size + 1, 1);

    ret = sprintf(r->out, "HTTP/1.1 %d %s\r\n", r->code, r->msg);
    (void)ret;
    p = r->out;

    if (r->code == 200 && r->body) {
        char content_length[10];
        sprintf(content_length, "%zd", r->body_len);
        http_response_set_header(r, "Content-Length", content_length);
    } else {
        http_response_set_header(r, "Content-Length", "0");
    }

    for (i = 0; i < r->header_count; ++i) {
        /* "Key: Value\r\n" */
        size_t header_size = r->headers[i].key_size + 2 + r->headers[i].val_size + 2;
        r->out = realloc(r->out, r->out_size + header_size);
        p = r->out + r->out_size;

        /* add key */
        memcpy(p, r->headers[i].key, r->headers[i].key_size);
        p += r->headers[i].key_size;

        /* add ": " */
        *(p++) = ':';
        *(p++) = ' ';

        /* add value */
        memcpy(p, r->headers[i].val, r->headers[i].val_size);
        p += r->headers[i].val_size;

        /* add "\r\n" */
        *(p++) = '\r';
        *(p++) = '\n';

        r->out_size += header_size;
    }

    /* end of headers */
    r->out = realloc(r->out, r->out_size + 2);
    memcpy(r->out + r->out_size, "\r\n", 2);
    r->out_size += 2;

    /* append body if there is one. */
    if (r->body && r->body_len) {
        char *tmp = (char*)r->body;
        size_t tmp_len = r->body_len;

        r->out = realloc(r->out, r->out_size + tmp_len);
        memcpy(r->out + r->out_size, tmp, tmp_len);
        r->out_size += tmp_len;
    }
}

void
http_response_free(struct http_response *r)
{
    if (r->out) free(r->out);

    while (r->header_count > 0) {
        r->header_count--;
        free(r->headers[r->header_count].key);
        free(r->headers[r->header_count].val);
    }

    free(r->headers);
    free(r);
}

char *
http_get_header_value(char *buffer, const char *key)
{
    char *b, *e, *value;

    b = strstr(buffer, key);
    if (b == NULL) return NULL;

    b += strlen(key) + 2;  // ': '

    e = strstr(b, "\r\n");
    if (e == NULL) return NULL;

    value = malloc(e-b+1);
    memset(value, 0, sizeof(value));
    memcpy(value, b, e-b);

    return value;
}
