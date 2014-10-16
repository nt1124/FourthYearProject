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
    int sockfd, newsockfd, portno, clilen;
    char *buffer;
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    
    if (argc < 2)
    {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        error("ERROR opening socket");
    }

    serv_addr = getSelfAsServer(argv[1]);

    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    {
        error("ERROR on binding");
    }

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) 
    {
        error("ERROR on accept");    
    }

    buffer = readFromSock(newsockfd);
    printf("Here is the message: %s\n",buffer);

    writeToSock(newsockfd, "I got your message", 18);

    return 0; 
}
