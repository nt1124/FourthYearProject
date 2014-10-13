#include "fileUtils.h"
#include "cryptoUtil.c"
#include "gateOrWire.cpp"
#include <stdio.h>
#include <stdlib.h>


class Circuit
{
  public:
	gateOrWire *gateList;
	int numGates;
	void readInCircuit(char* filepath, int numGates);
	void readInputLines(char *line);
	void runCircuit();
	void processGateLine(char *line, int *gateIndex);
	Circuit(char *filepath);
};


//gateOrWire
void Circuit::processGateLine(char *line, int *gateIndex)
{
	int strIndex = 0, idNum;

	while( line[strIndex] != ' ' )
	{
		if( '/' == line[strIndex] )
			return;
		strIndex ++;
	}

	char *idString = (char*) calloc(strIndex + 1, sizeof(char));
	strncpy(idString, line, strIndex);
	idNum = atoi(idString);

	while( line[strIndex] == ' ' )
	{
		if( '/' == line[strIndex] )
			return;
		strIndex ++;
	}

	// gateList[*gateIndex] =
	gateOrWire(line, idNum, &strIndex, gateList, gateIndex);
	// (*gateIndex) ++;
}


void Circuit::readInCircuit(char* filepath, int numGates)
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

			processGateLine(line, &gateIndex);
			// tempGateOrWire = processGateLine(line, circuit);
			// if( NULL != tempGateOrWire )
			// {
			// 	*(circuit + gateIndex) = tempGateOrWire;
			// 	gateIndex ++;
			// }
		}
		fclose ( file );
	}
}


void Circuit::readInputLines(char *line) //, struct gateOrWire **inputCircuit)
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
		gateList[gateID].wireValue = 1;
		gateList[gateID].wireEncValue = gateList[gateID].outputGarbleKeys -> key1;
	}
	else if( '0' == line[strIndex] )
	{
		gateList[gateID].wireValue = 0;
		gateList[gateID].wireEncValue = gateList[gateID].outputGarbleKeys -> key0;
	}
}

/*
void readInputDetailsFile(char *filepath, Circuit inputCircuit)
{
	FILE *file = fopen ( filepath, "r" );
	if ( file != NULL )
	{
		char line [ 512 ]; //Or other suitable maximum line size
		while ( fgets ( line, sizeof line, file ) != NULL )  //Read a line
		{
			inputCircuit.readInputLines(line);
		}

		fclose ( file );
	}
}
*/


Circuit::Circuit(char *filepath)
{
	FILE *file = fopen ( filepath, "r" );
	if ( file != NULL )
	{
		char line [ 512 ]; /* or other suitable maximum line size */
		while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
		{
			readInputLines(line);
		}

		fclose ( file );
	}
}


//struct gateOrWire **inputCircuit, int numGates )
void Circuit::runCircuit( )
{
	int i, j, tempIndex, numInputs;
	char outputTableIndex, tempValue;
	struct gate *currentGate;
	struct outputEncRow *outputTree;

	for(i = 0; i < numGates; i ++)
	{
		if( 'G' == gateList[i].typeTag )
		{
			outputTableIndex = 0;
			currentGate = gateList[i].gate_data;
			numInputs = currentGate -> numInputs;

			outputTree = gateList[i].gate_data -> outputTreeEnc;

			for(j = 0; j < numInputs; j ++)
			{
				tempIndex = currentGate -> inputIDs[numInputs - j - 1];
				outputTree = outputTree -> keyChoice[gateList[tempIndex].wireValue];
			}

			gateList[i].decryptionTree(gateList);
			gateList[i].wireValue = outputTree -> outputValue;
		}
	}
}