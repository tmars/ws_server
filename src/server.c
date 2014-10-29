#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include "client.h"
#include "websocket.h"

int 
main( int argc, char *argv[] )
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
    bzero((char *) &serv_addr, sizeof(serv_addr));
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
    listen(sockfd,5);
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
        }
        else {
            close(newsockfd);
        }
        //exit(1);
    } /* end of while */
}

void
doprocessing (struct ws_client *c)
{
    int n, i, num;
    n = process_handshake(c);
    if (n == 0) {
        perror("Error handshake");
        exit(1);
    }
    ws_client_remove_data(c);
    printf("Success handshake\n");
    while(1) {
        n = ws_client_read(c);
        if (n > 0) {
            char* data = parse_data(c->buffer, c->size);
            if (data != NULL) {
                printf("<<\n%s\n", data);
            }
            else {
                printf("error\n");
            }

            ws_client_remove_data(c);
        }
    }
    //exit(0);
}

