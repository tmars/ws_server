#include <stdlib.h>
#include "frame.h"

#ifndef CLIENT_H_
#define CLIENT_H_

struct ws_client {
    int sock;

    char *buffer;
    size_t size;
};

struct ws_client*
ws_client_new(int sock);

void
ws_client_remove_data(struct ws_client *c);

int
ws_client_read(struct ws_client *c);

int
ws_client_write(struct ws_client *c, char *data, size_t size);

struct frame *
ws_client_receive(struct ws_client *c);

int
ws_client_send(struct ws_client *c, struct frame *f);

#endif  // CLIENT_H_
