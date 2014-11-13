#ifndef SERVER_H_
#define SERVER_H_

#include "client.h"

struct server {
    int port;
    int sock;
    void (*on_client)(struct client *);
};

struct server *
server_new(int port);

void
server_start(struct server *s);

#endif  // SERVER_H_
