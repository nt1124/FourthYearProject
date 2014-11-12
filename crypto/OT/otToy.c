// Send
// Receive
// Pre-Compute
// Constructor to Init

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../comms/sockets.h"


void senderOT_Toy(int writeSocket, unsigned char *input0Bytes, unsigned char *input1Bytes, int inputLengths)
{
	unsigned char *bitRequested;
	int bitRequestLength = 0;

    bitRequested = receiveBoth(writeSocket, bitRequestLength);
	
	if(0x00 == *bitRequested)
		sendBoth(writeSocket, (octet*) input0Bytes, inputLengths);
	else if(0x01 == *bitRequested)
		sendBoth(writeSocket, (octet*) input1Bytes, inputLengths);

    free(bitRequested);
}



unsigned char *receiverOT_Toy(int writeSocket, unsigned char inputBit, int *outputLength)
{
	unsigned char *output;

	sendBoth(writeSocket, (octet*) &inputBit, 1);
    
    output = receiveBoth(writeSocket, *outputLength);

	return output;
}

/*
void testSender_OT_Toy(char *portNumStr)
{
    int sockfd, portNum;
    struct sockaddr_in serv_addr;
    
    portNum = atoi(portNumStr);
    sockfd = openSock();
    char *ipAddress = (char*) calloc(10, sizeof(char));
    strncpy(ipAddress, "127.0.0.1", 9);

    serv_addr = getServerAddr(ipAddress, portNum);
    connectToServer(&sockfd, serv_addr);

    unsigned char input0[17] = "1111111111111111";
    unsigned char input1[17] = "2222222222222222";

	senderOT_Toy(sockfd, input0, input1, 16);
}



void testReceiver_OT_Toy(char *portNumStr)
{

    int sockfd, newsockfd, clilen;
    struct sockaddr_in serv_addr, cli_addr;
    unsigned char *buffer;
    int bufferLength = 0, i;

    sockfd = openSock();
    serv_addr = getSelfAsServer(portNumStr);

    bindAndListenToSock(sockfd, serv_addr);
    clilen = sizeof(cli_addr);
    newsockfd = acceptNextConnectOnSock(sockfd, &cli_addr, &clilen);

    buffer = receiverOT_Toy(newsockfd, (char)1, &bufferLength);
    
    for(i = 0; i < bufferLength; i ++)
    {
    	printf("%c", buffer[i]);
    }
    printf("\n");
}
*/

/*
int main(int argc, char *argv[])
{
	int mode = atoi(argv[1]);

	if(0 == mode)
	{
		testSender_OT_Toy(argv[2]);
	}
	else
	{
		testReceiver_OT_Toy(argv[2]);
	}

	return 0;
}
*/