#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct server *
server_new(int port)
{
    struct server *s = calloc(1, sizeof(struct server));
    struct sockaddr_in serv_addr;

    s->port = port;
    s->on_client = NULL;

    /* First call to socket() function */
    s->sock = socket(AF_INET, SOCK_STREAM, 0);
    if (s->sock < 0) {
        perror("ERROR opening socket");
        return NULL;
    }

    /* Initialize socket structure */
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(s->port);

    /* Now bind the host address using bind() call.*/
    if (bind(s->sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        return NULL;
    }

    return s;
}

void
server_start(struct server *s)
{
    if (s == NULL) {
        return;
    }

    int client_sock, client_len;
    struct sockaddr_in client_addr;
    int pid;
    /* Now start listening for the clients, here 
     * process will go in sleep mode and will wait 
     * for the incoming connection
     */
    listen(s->sock, 5);
    client_len = sizeof(client_addr);
    while (1) {
        client_sock = accept(s->sock, (struct sockaddr *) &client_addr, &client_len);
        if (client_sock < 0) {
            perror("ERROR on accept");
            exit(1);
        }

        /* Create child process */
        pid = fork();
        if (pid < 0) {
            perror("ERROR on fork");
            exit(1);
        }

        if (pid == 0) {
            /* This is the client process */
            close(s->sock);
            struct client *c = client_new(client_sock);
            if (s->on_client != NULL) {
                (*s->on_client)(c);
            }
            client_work(c);
            exit(0);
        } else {
            close(client_sock);
        }
    }
}
