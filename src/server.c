#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "client.h"
#include "frame.h"

void
on_text_frame(struct ws_client *c, char *text, size_t size)
{
    printf("size=%d\ncontent:\n%s\n", size, text);
}

void
on_bin_frame(struct ws_client *c, char *data, size_t size)
{
    size_t i;
    printf("size=%d", size);
    for (i = 0; i < size; i++) {
        printf("0x%02x\n", data[i]);
    }
}

void
on_close(struct ws_client *c)
{
    printf("Connection closed.\n");
}

void
on_ping(struct ws_client *c)
{
    printf("PING\n");
}

int
main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int  n, pid;

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }
    /* Initialize socket structure */
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    portno = 4980;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
                          sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }
    /* Now start listening for the clients, here 
     * process will go in sleep mode and will wait 
     * for the incoming connection
     */
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    while (1) {
        newsockfd = accept(sockfd,
                (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
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
            close(sockfd);
            struct ws_client *c = ws_client_new(newsockfd);
            c->on_text_frame = &on_text_frame;
            c->on_bin_frame = &on_bin_frame;
            c->on_close = &on_close;
            c->on_ping = &on_ping;
            ws_client_work(c);
            exit(0);
        } else {
            close(newsockfd);
        }
        // exit(1);
    }
}
