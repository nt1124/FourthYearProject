#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sockets.h"



int main(int argc, char *argv[])
{
    sockaddr_in dest;
    int consocket, main_socket, length;
    int Portnum = atoi(argv[1]);
    char *buffer = (char*) calloc(40, sizeof(char));


    set_up_server_socket(dest, consocket, main_socket, Portnum);

    // receive(consocket, length);
    receiveBoth(consocket, (octet*)buffer, length);
    printf("Here is the message: %s\n", buffer);
    free(buffer);

    sendBoth(consocket, (octet*)"I got your message\0", strlen("I got your message\0"));

    close_server_socket(consocket, main_socket);
}


