#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "client.h"
#include "websocket.h"
#include "frame.h"

void
doprocessing(struct ws_client *c);

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
            doprocessing(c);
            exit(0);
        } else {
            close(newsockfd);
        }
        // exit(1);
    }
}

void
printbytes(const char t)
{
    int m;
    for (m = 128; m > 0; m = m >> 1) {
        printf("%u ", t & m ? 1 : 0);
    }
}

void
doprocessing(struct ws_client *c)
{
    int n, i, num;
    n = process_handshake(c);
    if (n == 0) {
        perror("Error handshake");
        exit(1);
    }
    ws_client_remove_data(c);
    printf("Success handshake\n");
    int ind = 0;
    while (1) {
        struct frame *r = ws_client_receive(c);
        if (r == NULL) {
            continue;
        }
        printf("<<%d\n%s\n", ind++, r->payload);
        frame_free(r);

        /*struct frame *s = frame_create(r->payload, r->payload_size, r->opcode);
        
        printf(">>\n%s\n", s->payload);
        ws_client_send(c, s);

        frame_free(s);
        frame_free(r);
        ws_client_remove_data(c);*/
    }
    // exit(0);
}
