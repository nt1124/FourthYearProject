#include "formatUtils.h"


void printAllOutput(struct gateOrWire **inputCircuit, int numGates)
{
	int i;

	for(i = 0; i < numGates; i ++)
	{
		if( 1 == inputCircuit[i] -> outputFlag )
		{
			printf("Gate %d = %d\n", inputCircuit[i] -> G_ID, inputCircuit[i] -> wireValue);
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

	while( ' ' != line[strIndex++] ){}

	while( ' ' != line[strIndex] )
	{
		gateID *= 10;
		curCharStr[0] = line[strIndex];
		gateID += atoi(curCharStr);
		strIndex ++;
	}
	strIndex ++;

	if( '1' == line[strIndex] )
	{
		inputCircuit[gateID] -> wireValue = 1;
		inputCircuit[gateID] -> wireEncValue = inputCircuit[gateID] -> outputGarbleKeys -> key1;
	}
	else if( '0' == line[strIndex] )
	{
		inputCircuit[gateID] -> wireValue = 0;
		inputCircuit[gateID] -> wireEncValue = inputCircuit[gateID] -> outputGarbleKeys -> key0;
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
	int i, j, tempIndex, numInputs;
	char outputTableIndex, tempValue;
	struct gate *currentGate;
	struct outputEncRow *outputTree;

	for(i = 0; i < numGates; i ++)
	{
		if( NULL != inputCircuit[i] -> gate_data )
		{
			outputTableIndex = 0;
			currentGate = inputCircuit[i] -> gate_data;
			numInputs = currentGate -> numInputs;

			outputTree = inputCircuit[i] -> gate_data -> outputTreeEnc;

			for(j = 0; j < numInputs; j ++)
			{
				tempIndex = currentGate -> inputIDs[numInputs - j - 1];
				outputTree = outputTree -> keyChoice[inputCircuit[tempIndex] -> wireValue];
			}

			inputCircuit[i] -> wireEncValue = decryptionTree(inputCircuit[i], inputCircuit);
			inputCircuit[i] -> wireValue = outputTree -> outputValue;
		}
	}
}