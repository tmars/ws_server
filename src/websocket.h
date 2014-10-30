#ifndef WEBSOCKET_H
#define WEBSOCKET_H

int 
process_handshake(struct ws_client *);

char *
parse_data(const char *, int);

char *
new_msg(const char *, int, int *);

#endif /* WEBSOCKET_H */