#include <string.h>
#include <assert.h>
#include "websocket.h"
#include "http.h"

int
main(int argc, char *argv[])
{
	char *buffer = 
		"GET / HTTP/1.1\r\n"
		"Host: localhost:4980\r\n"
		"Connection: Upgrade\r\n"
		"Pragma: no-cache\r\n"
		"Cache-Control: no-cache\r\n"
		"Upgrade: websocket\r\n"
		"Origin: file://\r\n"
		"Sec-WebSocket-Version: 13\r\n"
		"User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/38.0.2125.111 Safari/537.36\r\n"
		"Accept-Encoding: gzip,deflate,sdch\r\n"
		"Accept-Language: ru-RU,ru;q=0.8,en-US;q=0.6,en;q=0.4\r\n"
		"Sec-WebSocket-Key: 62pHtb5J9M43wbSH0xINSA==\r\n"
		"Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits\r\n";

	char *response = 
		"HTTP/1.1 101 Switching Protocols\r\n"
		"Upgrade: websocket\r\n"
		"Connection: Upgrade\r\n"
		"Sec-WebSocket-Accept: hYtlWCyrWbySXI6+cQa6oOSqGnE=\r\n"
		"Content-Length: 0\r\n\r\n";

	struct http_response *r = get_handshake_response(buffer);
	
	assert(r != NULL);
	assert(strcmp(r->out, response) == 0);
}