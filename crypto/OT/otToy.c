#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../comms/sockets.h"


void senderOT_Toy(int writeSocket, int readSocket, unsigned char *input0Bytes, unsigned char *input1Bytes, int inputLengths)
{
	unsigned char *bitRequested;
	int bitRequestLength = 0;

    bitRequested = receiveBoth(readSocket, bitRequestLength);
	
	if(0x00 == *bitRequested)
		sendBoth(writeSocket, (octet*) input0Bytes, inputLengths);
	else if(0x01 == *bitRequested)
		sendBoth(writeSocket, (octet*) input1Bytes, inputLengths);

    free(bitRequested);
}



unsigned char *receiverOT_Toy(int writeSocket, int readSocket, unsigned char inputBit, int *outputLength)
{
	unsigned char *output;

	sendBoth(writeSocket, (octet*) &inputBit, 1);
    
    output = receiveBoth(readSocket, *outputLength);

	return output;
}