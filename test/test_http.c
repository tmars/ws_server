#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "http.h"

int
main(int argc, char *argv[])
{
	struct http_response *r = http_response_init(101, "Switching Protocols");
	assert(r != NULL);

	http_response_set_header(r, "Upgrade", "websocket");
    http_response_set_header(r, "Connection", "Upgrade");
    http_response_pack(r);
	
	assert(r->out != NULL);
	assert(r->out_size != 0);
	
	char *response = 
		"HTTP/1.1 101 Switching Protocols\r\n"
		"Upgrade: websocket\r\n"
		"Connection: Upgrade\r\n"
		"Content-Length: 0\r\n\r\n";
	
	assert(strcmp(r->out, response) == 0);
}