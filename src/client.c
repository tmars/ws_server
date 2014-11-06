#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct ws_client*
ws_client_new(int sock)
{
    struct ws_client *c = calloc(1, sizeof(struct ws_client));

    c->sock = sock;

    return c;
}

void
ws_client_remove_data(struct ws_client *c)
{
    free(c->buffer);
    c->buffer = NULL;
    c->size = 0;
}

int
ws_client_read(struct ws_client *c)
{
    char buffer[4096];
    int n;

    n = read(c->sock, buffer, sizeof(buffer));
    if (n <= 0) {
        return -1;
    }

    c->buffer = realloc(c->buffer, c->size + n);
    if (!c->buffer) {
        return -1;
    }
    memcpy(c->buffer + c->size, buffer, n);
    c->size += n;

    return n;
}
