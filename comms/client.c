#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sockets.h"



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

    sendBoth(mysocket, (octet*) buffer, strlen(buffer));
    bzero(buffer, 257);

    receiveBoth(mysocket, (octet*)buffer, length);
    printf("%d --> %s\n", length, buffer);

    close_client_socket(mysocket);

    return 0;
}








