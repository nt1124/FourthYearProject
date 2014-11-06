#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "sockets.cpp"



int notMain(int argc, char *argv[])
{
    int sockfd, portNum, n;
    int length = 0;
    struct sockaddr_in serv_addr;
    char *buffer = (char*) calloc(257, sizeof(char));

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
    bzero(buffer, 257);
    fgets(buffer, 256, stdin);

    writeToSock(sockfd, buffer, 256);

    free(buffer);
    buffer = readFromSock(sockfd, &length);

    printf("%s\n", buffer);
    return 0;
}



int main(int argc, char *argv[])
{
    int mysocket, portNum, n;
    int length = 0;
    struct sockaddr_in dest;
    char *buffer = (char*) calloc(257, sizeof(char));
    portNum = atoi(argv[2]);

    set_up_client_socket(mysocket, argv[1], portNum, dest);

    printf("Please enter the message: ");
    bzero(buffer, 257);
    fgets(buffer, 256, stdin);
    writeToSock(sockfd, buffer, 256);
    free(buffer);

    buffer = readFromSock(sockfd, &length);

    printf("%s\n", buffer);

    close_client_socket(mysocket);

    return 0;
}




