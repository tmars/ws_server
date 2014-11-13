#include <stdlib.h>
#include "frame.h"

#ifndef CLIENT_H_
#define CLIENT_H_

struct client {
    int sock;

    char *buffer;
    size_t size;

    void (*on_text_frame)(struct client *, char *, size_t);
    void (*on_bin_frame)(struct client *, char *, size_t);
    void (*on_close)(struct client *);
    void (*on_ping)(struct client *);
};

struct client*
client_new(int sock);

void
client_remove_data(struct client *c);

int
client_read(struct client *c);

int
client_write(struct client *c, char *data, size_t size);

struct frame *
client_receive(struct client *c);

int
client_send(struct client *c, struct frame *f);

void
client_work(struct client *c);

#endif  // CLIENT_H_
