#ifndef WEBSOCKET_H
#define WEBSOCKET_H

char *
get_header_value(char *, const char *);

char *
get_handshake_hash(const char *);

int 
process_handshake(struct ws_client *);

char *
parse_data(const char *, int);

#endif /* WEBSOCKET_H */