#ifndef CLIENT_H_
#define CLIENT_H_

struct ws_client {
    int sock;

    char *buffer;
    size_t size;
};

struct ws_client*
ws_client_new(int sock);

int
ws_client_remove_data(struct ws_client *);

int
ws_client_read(struct ws_client *);

#endif  // CLIENT_H_
