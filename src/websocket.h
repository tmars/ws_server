#ifndef WEBSOCKET_H_
#define WEBSOCKET_H_

#include "http.h"

struct http_response *
get_handshake_response(char *header);

#endif  // WEBSOCKET_H_
