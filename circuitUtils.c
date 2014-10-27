#include "formatUtils.h"


void printAllOutput(struct gateOrWire **inputCircuit, int numGates)
{
	int i;

	for(i = 0; i < numGates; i ++)
	{
		if( 0x0F == inputCircuit[i] -> outputWire -> wireMask )
		{
			printf("Gate %d = %d\n", inputCircuit[i] -> G_ID, inputCircuit[i] -> outputWire -> wirePermedValue);
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
			printf("%d ------------------------\n", gateIndex);
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
	if( '1' == line[strIndex] )
	{
		outputWire -> wirePermedValue = 0x01;
		outputWire -> wireOutputKey = outputWire -> outputGarbleKeys -> key1;
	}
	else if( '0' == line[strIndex] )
	{
		outputWire -> wirePermedValue = 0x00;
		outputWire -> wireOutputKey = outputWire -> outputGarbleKeys -> key0;
	}
}


void readInputDetailsFile(char *filepath, struct gateOrWire **inputCircuit)
{
	FILE *file = fopen ( filepath, "r" );
	if ( file != NULL )
	{
		char line [ 512 ]; /* or other suitable maximum line size */
		while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
		{
			readInputLines(line, inputCircuit);
		}
		fclose ( file );
	}
}


void runCircuit( struct gateOrWire **inputCircuit, int numGates )
{
	int i, j, k, tempIndex, numInputs;
	char outputTableIndex, tempBit;
	struct gate *currentGate;
	unsigned char *outputChars;
	unsigned char *tempEncValue;

	for(i = 0; i < numGates; i ++)
	{
		if( NULL != inputCircuit[i] -> gatePayload )
		{
			outputTableIndex = 0;
			currentGate = inputCircuit[i] -> gatePayload;
			numInputs = currentGate -> numInputs;

			for(j = 0; j < numInputs; j ++)
			{
				tempIndex = currentGate -> inputIDs[numInputs - j - 1];
				tempBit = inputCircuit[tempIndex] -> outputWire -> wirePermedValue;
				outputTableIndex <<= 1;
				outputTableIndex += tempBit;
			}
			outputChars = inputCircuit[i] -> gatePayload -> encOutputTable[outputTableIndex];

			// inputCircuit[i] -> outputWire -> wireEncValue = decryptionTree(inputCircuit[i], inputCircuit);
			if( 1 == inputCircuit[i] -> outputWire -> wireMask )
			{
				for(k = 0; k < inputCircuit[i] -> gatePayload -> outputTableSize; k ++)
				{
					/*
					if( 0 == strncpy(inputCircuit[i] -> wireEncValue, , 16) )
					{

					}
					*/
				}
				inputCircuit[i] -> outputWire -> wirePermedValue = outputChars[16];
			}
		}
	}
}