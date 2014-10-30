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


void readInputLines(char *line, struct gateOrWire **inputCircuit)
{
	int strIndex = 0, gateID = 0, wireValue;
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
	inputCircuit[gateID] -> outputWire -> wireOwner = 0xFF;
	if( '1' == line[strIndex] )
	{
		outputWire -> wirePermedValue = outputWire -> outputGarbleKeys -> key1[16];
		outputWire -> wireOutputKey = outputWire -> outputGarbleKeys -> key1;
	}
	else if( '0' == line[strIndex] )
	{
		outputWire -> wirePermedValue = outputWire -> outputGarbleKeys -> key0[16];
		outputWire -> wireOutputKey = outputWire -> outputGarbleKeys -> key0;
	}
}


void readInputDetailsFile(char *filepath, struct gateOrWire **inputCircuit)
{
	FILE *file = fopen ( filepath, "r" );
	if ( file != NULL )
	{
		char line [ 512 ];
		while ( fgets ( line, sizeof line, file ) != NULL )
		{
			readInputLines(line, inputCircuit);
		}
		fclose ( file );
	}
}


void runCircuitExecutor( struct gateOrWire **inputCircuit, int numGates, int sockfd )
{
	int i;

	for(i = 0; i < numGates; i ++)
	{
		if( NULL != inputCircuit[i] -> gatePayload )
		{
			decryptGate(inputCircuit[i], inputCircuit, sockfd);
		}
	}
}


void runCircuitBuilder( struct gateOrWire **inputCircuit, int numGates, int sockfd )
{
	int i;

	for(i = 0; i < numGates; i ++)
	{
		if( NULL != inputCircuit[i] -> gatePayload )
		{
			provideKeyForGate(inputCircuit[i], sockfd);
		}
	}
}