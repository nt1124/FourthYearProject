/* A simple server in the internet domain using TCP
   The port number is passed as an argument */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "socketUtils.c"


int main(int argc, char *argv[])
{
    int sockfd, newsockfd, clilen;
    int length = 0;
    char *buffer;
    struct sockaddr_in serv_addr, cli_addr;

    
    if (argc < 2)
    {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    sockfd = openSock();
    serv_addr = getSelfAsServer(argv[1]);

    bindAndListenToSock(sockfd, serv_addr);
    clilen = sizeof(cli_addr);
    newsockfd = acceptNextConnectOnSock(sockfd, &cli_addr, &clilen);

    buffer = readFromSock(newsockfd, &length);
    printf("Here is the message: %s\n",buffer);

    writeToSock(newsockfd, "I got your message", 18);

    return 0;
}
