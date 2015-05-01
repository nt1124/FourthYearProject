#ifndef _sockets
#define _sockets


long long unsigned int byteSendCount = 0;
long long unsigned int byteReceivedCount = 0;

long long unsigned int subSendCount = 0;
long long unsigned int subReceivedCount = 0;

long long unsigned int totalSendCount = 0;
long long unsigned int totalReceivedCount = 0;

#include "data.h"

#include <errno.h>      /* Errors */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <arpa/inet.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>   /* Wait for Process Termination */
#include "data.cpp"



void error(const char *str1,const char *str2);
void error(const char *str);

void set_up_server_socket(sockaddr_in& dest, int& consocket, int& main_socket, int Portnum);
void close_server_socket(int consocket, int main_socket);

void set_up_client_socket(int& mysocket, const char* hostname, int Portnum, sockaddr_in& dest);
void close_client_socket(int socket);

void sendBoth(int socket, octet *msg, int len);
unsigned char *receiveBoth(int socket, int& len);

void send(int socket, octet *msg, int len);
void receive(int socket, octet *msg, int len);

/* Send and receive 8 bit integers */
void send(int socket, int a);
void receive(int socket, int& a);

void sendInt(int socket, int input);
int receiveInt(int socket);

void send_ack(int socket);
int get_ack(int socket);


#include "sockets.cpp"

#endif
