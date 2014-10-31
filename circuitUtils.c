#include "formatUtils.h"


void printAllOutput(struct gateOrWire **inputCircuit, int numGates)
{
	int i;
	unsigned char tempBit;

	for(i = 0; i < numGates; i ++)
	{
		if( 0x0F == inputCircuit[i] -> outputWire -> wireMask )
		{
			tempBit = inputCircuit[i] -> outputWire -> wirePermedValue;
			tempBit = tempBit ^ (0x01 & inputCircuit[i] -> outputWire -> wirePerm);
			printf("Gate %d = %d\n", inputCircuit[i] -> G_ID, tempBit);
		}
	}
}


struct gateOrWire *processGateLine(char *line, struct gateOrWire **circuit)
{
	int strIndex = 0, idNum;

	while( line[strIndex] != ' ' )
	{
		if( '/' == line[strIndex] )
			return NULL;
		strIndex ++;
	}

	char *idString = (char*) calloc(strIndex + 1, sizeof(char));
	strncpy(idString, line, strIndex);
	idNum = atoi(idString);

	while( line[strIndex] == ' ' )
	{
		if( '/' == line[strIndex] )
			return NULL;
		strIndex ++;
	}

	return processGateOrWire(line, idNum, &strIndex, circuit);
}


struct gateOrWire **readInCircuit(char* filepath, int numGates)
{
	int gateIndex = 0;
	struct gateOrWire *tempGateOrWire;
	struct gateOrWire **circuit = (struct gateOrWire**) calloc(numGates, sizeof(struct gateOrWire*));

	FILE *file = fopen ( filepath, "r" );
	if ( file != NULL )
	{
		char line [ 512 ]; /* or other suitable maximum line size */
		while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
		{
			tempGateOrWire = processGateLine(line, circuit);
			if( NULL != tempGateOrWire )
			{
				*(circuit + gateIndex) = tempGateOrWire;
				gateIndex ++;
			}
		}
		fclose ( file );
	}

	return circuit;
}



void readInputLinesBuilder(char *line, struct gateOrWire **inputCircuit)
{
	int strIndex = 0, gateID = 0;
	char *curCharStr = (char*) calloc( 2, sizeof(char) );
	struct wire *outputWire;

	while( ' ' != line[strIndex++] ){}

	while( ' ' != line[strIndex] )
	{
		gateID *= 10;
		curCharStr[0] = line[strIndex];
		gateID += atoi(curCharStr);
		strIndex ++;
	}
	strIndex ++;

	outputWire = inputCircuit[gateID] -> outputWire;
	outputWire -> wireOwner = 0xFF;
	if( '1' == line[strIndex] )
	{
		outputWire -> wirePermedValue = outputWire -> outputGarbleKeys -> key1[16];
		memcpy(outputWire -> wireOutputKey, outputWire -> outputGarbleKeys -> key1, 16);
	}
	else if( '0' == line[strIndex] )
	{
		outputWire -> wirePermedValue = outputWire -> outputGarbleKeys -> key0[16];
		memcpy(outputWire -> wireOutputKey, outputWire -> outputGarbleKeys -> key0, 16);
	}
}


void readInputDetailsFileBuilder(char *filepath, struct gateOrWire **inputCircuit)
{
	FILE *file = fopen ( filepath, "r" );
	if ( file != NULL )
	{
		char line [ 512 ];
		while ( fgets ( line, sizeof line, file ) != NULL )
		{
			readInputLinesBuilder(line, inputCircuit);
		}
		fclose ( file );
	}
}



void readInputLinesExec(char *line, struct gateOrWire **inputCircuit, int sockfd)
{
	int strIndex = 0, gateID = 0, outputLength = 0;
	char *curCharStr = (char*) calloc( 2, sizeof(char) );
	struct wire *outputWire;

	while( ' ' != line[strIndex++] ){}
	while( ' ' != line[strIndex] )
	{
		gateID *= 10;
		curCharStr[0] = line[strIndex];
		gateID += atoi(curCharStr);
		strIndex ++;
	}
	strIndex ++;

	outputWire = inputCircuit[gateID] -> outputWire;

	if( '1' == line[strIndex] )
	{
		outputWire -> wireOutputKey = receiverOT_Toy(sockfd, (unsigned char)0x01, &outputLength);
		outputWire -> wirePermedValue = 0x01 ^ (outputWire -> wirePerm & 0x01);
	}
	else if( '0' == line[strIndex] )
	{
		outputWire -> wireOutputKey = receiverOT_Toy(sockfd, (unsigned char)0x00, &outputLength);
		outputWire -> wirePermedValue = 0x00 ^ (outputWire -> wirePerm & 0x01);
	}
	// printf("Received key for Gate %d\n", gateID);
}


void readInputDetailsFileExec(char *filepath, struct gateOrWire **inputCircuit, int sockfd)
{
	FILE *file = fopen ( filepath, "r" );
	if ( file != NULL )
	{
		char line [ 512 ];
		while ( fgets ( line, sizeof line, file ) != NULL )
		{
			readInputLinesExec(line, inputCircuit, sockfd);
		}
		fclose ( file );
	}
}



void runCircuitExec( struct gateOrWire **inputCircuit, int numGates, int sockfd, char *filepath )
{
	int i;

	readInputDetailsFileExec(filepath, inputCircuit, sockfd);

	for(i = 0; i < numGates; i ++)
	{
		printf("++++++++ %d\n", i);
		if( NULL != inputCircuit[i] -> gatePayload )
		{
			decryptGate(inputCircuit[i], inputCircuit);
		}
	}
}


void runCircuitBuilder( struct gateOrWire **inputCircuit, int numGates, int sockfd )
{
	int i;

	for(i = 0; i < numGates; i ++)
	{
		if( 0x00 == inputCircuit[i] -> outputWire -> wireOwner &&
			0xF0 == inputCircuit[i] -> outputWire -> wireMask )
		{
			// printf("Providing key for Gate %d\n", i);
			provideKeyForGate(inputCircuit[i], sockfd);
		}
	}
}


void runCircuitLocal( struct gateOrWire **inputCircuit, int numGates )
{
	int i;

	for(i = 0; i < numGates; i ++)
	{
		if( NULL != inputCircuit[i] -> gatePayload )
		{
			decryptGate(inputCircuit[i], inputCircuit);
		}
	}
}



void sendGate(struct gateOrWire *inputGW, int sockfd)
{
	unsigned char *buffer;
	int bufferLength;

	buffer = serialiseGateOrWire(inputGW, &bufferLength);
	printf("Sending the %dth gate. Size is %d\n", inputGW -> G_ID, bufferLength);
	
	// writeToSock(sockfd, (char*)buffer, bufferLength);
}


void sendCircuit(struct gateOrWire **inputCircuit, int numGates, int sockfd)
{
	int i;
	char *buffer = (char*) calloc(5, sizeof(char));

	memcpy(buffer, &numGates, 4);
	writeToSock(sockfd, buffer, 4);
	//free(buffer);

	for(i = 0; i < 1; i ++)// numGates; i ++)
	{
		sendGate(inputCircuit[i], sockfd);
		printf("%dth gate has been sent.\n", i);
		fflush(stdout);
	}
	printf("Circuit sent.\n");
}


int receiveNumGates(int sockfd)
{
	int numGates, bufferLength = 0;
	unsigned char *buffer;

	buffer = (unsigned char*) readFromSock(sockfd, &bufferLength);
	memcpy(&numGates, buffer, 4);
	free(buffer);

	return numGates;
}


struct gateOrWire **receiveCircuit(int numGates, int sockfd)
{
	int i, bufferLength = 0;
	unsigned char *buffer = NULL;
	struct gateOrWire **inputCircuit;

	inputCircuit = (struct gateOrWire **) calloc(numGates, sizeof(struct gateOrWire*));

	printf("Circuit has %d gates!\n", numGates);

	for(i = 0; i < 1; i ++)//numGates; i ++)
	{
		buffer = (unsigned char*) readFromSock(sockfd, &bufferLength);
		printf("Received the %dth gate. Size was %d\n", i, bufferLength);
		inputCircuit[i] = deserialiseGateOrWire(buffer);
		free(buffer);
	}
	printf("Circuit received.\n");

	return inputCircuit;
}