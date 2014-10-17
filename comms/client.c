#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "socketUtils.c"



int main(int argc, char *argv[])
{
    int sockfd, portNum, n;
    struct sockaddr_in serv_addr;
    char *buffer = calloc(256, sizeof(char));

    if (argc < 3)
    {
       fprintf(stderr, "usage %s ip address, port\n", argv[0]);
       exit(0);
    }
    
    portNum = atoi(argv[2]);
    sockfd = openSock();

    serv_addr = getServerAddr(argv[1], portNum);
    connectToServer(&sockfd, serv_addr);
    
    printf("Please enter the message: ");
    bzero(buffer, 256);
    fgets(buffer, 255, stdin);

    n = writeToSock(sockfd, buffer, 255);
    if (n < 0)
    {
         error("ERROR writing to socket");
    }

    free(buffer);
    buffer = readFromSock(sockfd);

    printf("%s\n", buffer);
    return 0;
}
