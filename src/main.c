#include <stdio.h>
#include <sys/types.h>
#include "client.h"
#include "server.h"

void
on_text_frame(struct client *c, char *text, size_t size)
{
    printf("size=%d\ncontent:\n%s\n", size, text);
}

void
on_bin_frame(struct client *c, char *data, size_t size)
{
    size_t i;
    printf("size=%d\ncontent:\n", size);
    //for (i = 0; i < size; i++) {
    //    printf("0x%02x\n", data[i]);
    //}
}

void
on_close(struct client *c)
{
    printf("Connection closed.\n");
}

void
on_ping(struct client *c)
{
    printf("PING\n");
}

void
on_client(struct client *c)
{
    printf("Connection opened.\n");

    c->on_text_frame = &on_text_frame;
    c->on_bin_frame = &on_bin_frame;
    c->on_close = &on_close;
    c->on_ping = &on_ping;
}

int
main(int argc, char *argv[])
{
    struct server *s = server_new(4980);
    if (s == NULL) {
        printf("Creating server: error\n");
        exit(1);
    }

    s->on_client = &on_client;
    server_start(s);
}
