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
	void readInputLines(char *line, int skipIndex);
	void runCircuit();
	void processGateLine(char *line, int *gateIndex);
	void printAllOutput();
	void getInputKeys();

	Circuit(char *filepath, int numGates);
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
	(*gateIndex) ++;
}


void Circuit::readInCircuit(char* filepath, int numGates)
{
	int gateIndex = 0;

	FILE *file = fopen ( filepath, "r" );
	if ( file != NULL )
	{
		char line [ 512 ]; /* or other suitable maximum line size */
		while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
		{
			processGateLine(line, &gateIndex);
		}
		fclose ( file );
	}
}


void Circuit::readInputLines(char *line, int skipIndex) //, struct gateOrWire **inputCircuit)
{
	int strIndex = 0, gateID = 0, wireValue;
	char *curCharStr = (char*) calloc( 2, sizeof(char) );

	while( ' ' != *(line + strIndex) )
	{
		strIndex ++;
	}
	strIndex ++;

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


void readInputDetailsFile(char *filepath, Circuit inputCircuit, int skipIndex)
{
	FILE *file = fopen ( filepath, "r" );
	int i;
	
	if ( file != NULL )
	{
		char line [ 512 ]; //Or other suitable maximum line size
		while ( fgets ( line, sizeof line, file ) != NULL )  //Read a line
		{
			inputCircuit.readInputLines(line, skipIndex);
		}

		fclose ( file );
	}
}



void Circuit::getInputKeys()
{
	int i, j;

	for(i = 0; i < numGates; i ++)
	{
		if('G' == gateList[i].typeTag)
		{
			gateList[i].getInputGarbleKeys(gateList);
		}
		gateList[i].recursiveEncryptionTree();
	}
}


Circuit::Circuit(char *filepath, int inputNumGates)
{
	int gateIndex = 0;
	numGates = inputNumGates;
	gateList = (struct gateOrWire*) calloc(numGates, sizeof(struct gateOrWire*));
	FILE *file = fopen ( filepath, "r" );

	if(file != NULL)
	{
		char line[512]; /* or other suitable maximum line size */
		while ( fgets(line, sizeof line, file) != NULL ) /* read a line */
		{
				printf("%d >>>>>>>>   %s", sizeof(line), line);
				processGateLine(line, &gateIndex);
				// readInputLines(line);
		}
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


void Circuit::printAllOutput()
{
	int i;

	for(i = 0; i < numGates; i ++)
	{
		if( 1 == gateList[i].outputFlag )
		{
			printf("Gate %d = %d\n", gateList[i].G_ID, gateList[i].wireValue);
		}
	}
}
