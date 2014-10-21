#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/*  -+ USING THIS MINI-LIBRARY +-

    -+ SERVER +-

    sockfd = openSock();
    serv_addr = getSelfAsServer(argv[1]);

    bindAndListenToSock(sockfd, serv_addr);
    clilen = sizeof(cli_addr);
    newsockfd = acceptNextConnectOnSock(sockfd, &cli_addr, &clilen);

    Then use the sockfd as the Sockets identifier.


    -+ CLIENT +-

    sockfd = openSock();
    
    serv_addr = getServerAddr(ipAddress, portNum);
    OR you can get serv_addr with 
    serv_addr = getServerAddrByHostname(hostname, portNum);

    connectToServer(&sockfd, serv_addr);

    -+ READ/WRITE ON A CONNECTED SOCKET +-
        Where sockfd is the file descriptor for the open socket.

    writeToSock(sockfd, buffer, bufferLength);
    char *readFromSocket = readFromSock(sockfd);

*/


void error(char *msg)
{
    printf("WELL THIS SUCKS!\n");
    perror(msg);
    exit(0);
}


int bytesToInteger(unsigned char *input)
{
    int output = 0, i;

    for(i = 0; i < sizeof(int); i ++)
    {
        output += (input[i] << (8 * i));
    }

    return output;
}


unsigned char *integerToBytes(int input)
{
    unsigned char *output = (unsigned char *) calloc(sizeof(int), sizeof(unsigned char));
    int i;
    int localInput = input;

    for(i = 0; i < sizeof(int); i ++)
    {
        output[i] = (unsigned char) localInput & 0x000000FF;
        localInput >>= 8;
    }

    return output;
}


int writeToSock(int sockfd, char *buffer, int bufferLength)
{
    int n = 0;
    unsigned char *bufferOfLength = (unsigned char*) calloc(sizeof(int) + 1, sizeof(char));

    bufferOfLength = integerToBytes(bufferLength);

    n = write(sockfd, bufferOfLength, sizeof(int));
    if (n < 0)
        error("ERROR writing to socket");
    printf("n = %d\n", n);

    n = write(sockfd, buffer, bufferLength);
    if (n < 0)
        error("ERROR writing to socket");

    return 1;
}


// Need to convert lengthAsBytes to an int.
// Then calloc memory to hold lengthAsBytes many bytes.
char *readFromSock(int sockfd)
{
    char *buffer;
    unsigned char *lengthAsBytes = (unsigned char*) calloc(sizeof(int), sizeof(unsigned char));
    int n = 0;
    unsigned int bufferLength = 255;


    n = read(sockfd, lengthAsBytes, sizeof(int));
    if (n < 0)
         error("ERROR reading from socket");

    for(n = 0; n < 4; n ++)
        printf("%u\n", lengthAsBytes[n]);

    bufferLength = bytesToInteger( (unsigned char*) lengthAsBytes);
    buffer = calloc(bufferLength + 1, sizeof(char));

    n = read(sockfd, buffer, bufferLength);    
    if (n < 0)
         error("ERROR reading from socket");
    printf("Read this many bytes = %d\n", bufferLength + 1);

    return buffer;
}


int openSock()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        error("ERROR opening socket");
    }

    return sockfd;
}


void connectToServer(int *sockfd, struct sockaddr_in serv_addr)
{
    if( connect(*sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0 ) 
    {
        error("ERROR connecting");
    }
}


struct sockaddr_in getSelfAsServer(char *portNumChars)
{
    struct sockaddr_in serv_addr;
    int portNum = atoi(portNumChars);

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portNum);

    return serv_addr;
}


struct sockaddr_in getServerAddrByHostname(char *hostname, int portNum)
{
    struct sockaddr_in serv_addr;
    struct hostent *server;

    server = gethostbyname(hostname);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero( (char *) &serv_addr, sizeof(serv_addr) );
    serv_addr.sin_family = AF_INET;
    bcopy((char *) server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
                   server->h_length);
    serv_addr.sin_port = htons(portNum);

    return serv_addr;
}


struct sockaddr_in getServerAddr(char *ipAddress, int portNum)
{
    struct sockaddr_in serv_addr;

    bzero( (char *) &serv_addr, sizeof(serv_addr) );
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr( ipAddress );
    serv_addr.sin_port = htons(portNum);

    return serv_addr;
}


void bindAndListenToSock(int sockfd, struct sockaddr_in serv_addr)
{
    if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    {
        error("ERROR on binding");
    }

    listen(sockfd, 5);
}


int acceptNextConnectOnSock(int sockfd, struct sockaddr_in *cli_addr, int *clilen)
{
    int newsockfd = accept(sockfd, (struct sockaddr*)cli_addr, clilen);
    if (newsockfd < 0) 
    {
        error("ERROR on accept");    
    }

    return newsockfd;
}



