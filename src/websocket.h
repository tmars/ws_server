#ifndef WEBSOCKET_H
#define WEBSOCKET_H

char *
get_header_value(char *, const char *);

char *
get_handshake_hash(const char *);

int 
process_handshake(struct ws_client *);

#endif /* WEBSOCKET_H */