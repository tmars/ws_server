#ifndef HTTP_H_
#define HTTP_H_

#include <sys/types.h>  // size_t

struct http_header {
    char *key;
    size_t key_size;

    char *val;
    size_t val_size;
};

struct http_response {
    int16_t code;
    const char *msg;

    struct http_header *headers;
    int header_count;

    const char *body;
    size_t body_len;

    char *out;
    size_t out_size;
};

struct http_response *
http_response_init(int16_t code, const char *msg);

void
http_response_set_header(struct http_response *r, const char *k, const char *v);

void
http_response_pack(struct http_response *r);

void
http_response_free(struct http_response *r);

char *
http_get_header_value(char *buffer, const char *key);

#endif  // HTTP_H_
