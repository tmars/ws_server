#ifndef CLIENT_H
#define CLIENT_H

struct ws_client {
	int sock;

	char *buffer;
	size_t size;
};

struct ws_client* 
ws_client_new(int);

int 
ws_client_remove_data(struct ws_client *);

int 
ws_client_read(struct ws_client *);

#endif /* CLIENT_H */