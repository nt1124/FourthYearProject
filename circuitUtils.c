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



struct idAndValue *readInputLinesExec(char *line, struct gateOrWire **inputCircuit, int sockfd)
{
	struct idAndValue *toReturn = (struct idAndValue*) calloc(1, sizeof(struct idAndValue));
	int strIndex = 0, gateID = 0, outputLength = 0, i;
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

	toReturn -> id = gateID;
	if( '1' == line[strIndex] )
	{
		toReturn -> value = 1;
	}
	else if( '0' == line[strIndex] )
	{
		toReturn -> value = 0;
	}



	return toReturn;
}


struct idAndValue *readInputDetailsFileExec(char *filepath, struct gateOrWire **inputCircuit, int sockfd)
{
	struct idAndValue *start = (struct idAndValue*) calloc(1, sizeof(struct idAndValue));
	struct idAndValue *insertion = start;
	FILE *file = fopen ( filepath, "r" );

	if ( file != NULL )
	{
		char line [ 512 ];
		while ( fgets ( line, sizeof line, file ) != NULL )
		{
			insertion -> next = readInputLinesExec(line, inputCircuit, sockfd);
			insertion = insertion -> next;
		}
		fclose ( file );
	}

	return start;
}



void runCircuitExec( struct gateOrWire **inputCircuit, int numGates, int sockfd, char *filepath )
{
	struct idAndValue *start;
	int i, outputLength = 0, j;

	start = readInputDetailsFileExec(filepath, inputCircuit, sockfd);

	while(NULL != start -> next)
	{
		start = start -> next;
		i = start -> id;
		inputCircuit[i] -> outputWire -> wirePermedValue = start -> value ^ (inputCircuit[i] -> outputWire -> wirePerm & 0x01);
		inputCircuit[i] -> outputWire -> wireOutputKey = receiverOT_Toy(sockfd, start -> value, &outputLength);
	}

	for(i = 0; i < numGates; i ++)
	{
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
	int bufferLength, j;

	buffer = serialiseGateOrWire(inputGW, &bufferLength);
	//printf("Sending the %dth gate. Size is %d\n", inputGW -> G_ID, bufferLength);
	
	writeToSock(sockfd, (char*)buffer, bufferLength);
	
	if(NULL != inputGW -> gatePayload)
	{
		for(j = 0; j < inputGW -> gatePayload -> outputTableSize; j ++)
		{
			writeToSock(sockfd, (char*)inputGW -> gatePayload -> encOutputTable[j], 32);
		}
	}
}


void sendCircuit(struct gateOrWire **inputCircuit, int numGates, int sockfd)
{
	int i;
	char *buffer = (char*) calloc(5, sizeof(char));

	memcpy(buffer, &numGates, 4);
	writeToSock(sockfd, buffer, 4);
	free(buffer);

	for(i = 0; i < numGates; i ++)
	{
		sendGate(inputCircuit[i], sockfd);
		// printf("%dth gate has been sent.\n", i);
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
	int i, j, bufferLength = 0;
	unsigned char *buffer = NULL;
	struct gateOrWire **inputCircuit;

	inputCircuit = (struct gateOrWire **) calloc(numGates, sizeof(struct gateOrWire*));

	printf("Circuit has %d gates!\n", numGates);

	for(i = 0; i < numGates; i ++)
	{
		// printf("Received the %dth gate. Size was %d\n", i, bufferLength);
		buffer = (unsigned char*) readFromSock(sockfd, &bufferLength);
		inputCircuit[i] = deserialiseGateOrWire(buffer);

		if(NULL != inputCircuit[i] -> gatePayload)
		{
			for(j = 0; j < inputCircuit[i] -> gatePayload -> outputTableSize; j ++)
			{
				inputCircuit[i] -> gatePayload -> encOutputTable[j] = (unsigned char*) readFromSock(sockfd, &bufferLength);
			}
		}
	}

	return inputCircuit;
}