#include "fileUtils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/rand.h>



typedef struct outputEncRow
{
	struct outputEncRow *keyChoice[2];

	unsigned char outputValue;
	unsigned char *outputEncValue;
} outputEncRow;


typedef struct bitsGarbleKeys
{
	unsigned char key0[16];
	unsigned char key1[16];
} bitsGarbleKeys;


typedef struct gate
{
	int numInputs;
	int *inputIDs;

	int outputTableSize;
	struct outputEncRow *outputTreeEnc;
	unsigned char *outputTable[16];

	struct bitsGarbleKeys **inputKeySet;
} gate;


class gateOrWire
{
	public:
		int G_ID;
		char wireValue;
		unsigned char *wireEncValue;
		char outputFlag;
		char typeTag;

		struct bitsGarbleKeys *outputGarbleKeys;
		struct gate *gate_data;

		void recursiveEncryptionTree();
		void decryptionTree(struct gateOrWire *inputCircuit);
		void generateGarbleKeyPair();
		void getInputGarbleKeys(gateOrWire *circuit, int *inputIDs);
		void printGateOrWire();
		void processGateOrWire(char *line, int idNum, int *strIndex);
		gateOrWire(char *line, int idNum, int *strIndex, gateOrWire *gateList, int *gateIndex);
};



struct gate *processGate(char* line, int strIndex, struct gateOrWire *curGate)
{
	struct gate *toReturn = (struct gate*) calloc(1, sizeof(struct gate));
	int tempIndex, outputTableSize = 1, *tableToParse;

	while( line[++ strIndex] != ' ' ) {}
	while( line[++ strIndex] != ' ' ) {}
	tempIndex = ++ strIndex;
	while( line[strIndex] != ' ' ) { strIndex ++; }

	char *tempString = (char*) calloc((strIndex - tempIndex + 1), sizeof(char));
	strncpy(tempString, line + tempIndex, (strIndex - tempIndex));
	toReturn -> numInputs = atoi(tempString);

	for(tempIndex = 0; tempIndex < toReturn -> numInputs; tempIndex ++)
	{
		outputTableSize *= 2;
	}

	toReturn -> outputTableSize = outputTableSize;
	tableToParse = parseOutputTable(line, &strIndex, toReturn);
	toReturn -> inputIDs = parseInputTable(line, toReturn -> numInputs, &strIndex);
	// toReturn -> inputKeySet = getInputGarbleKeys(toReturn -> numInputs, toReturn -> inputIDs);
	toReturn -> outputTreeEnc = recursiveOutputTable(tableToParse, toReturn);
	
	return toReturn;
}

/*
void gateOrWire::processGateOrWire(char *line, int idNum, int *strIndex)
{
	struct gateOrWire *toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));
	int i;

	G_ID = idNum;
	generateGarbleKeyPair();

	if( 'i' == line[*strIndex] )
	{
		typeTag = 'W';
	}
	else
	{
		typeTag = 'G';
		if('o' == line[*strIndex])
		{
			toReturn -> outputFlag = 1;
			while( line[*strIndex] != ' ' )
			{
				*strIndex = *strIndex + 1;
			}
		}

		//toReturn -> gate_data = processGate(line, *strIndex, circuit, toReturn);
		gate_data = processGate(line, *strIndex, toReturn);
		recursiveEncryptionTree();
	}
}
*/

gateOrWire::gateOrWire(char *line, int idNum, int *strIndex, gateOrWire *gateList, int *gateIndex)
{
	struct gateOrWire *toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));
	int i;

	G_ID = idNum;
	generateGarbleKeyPair();

	if( 'i' == line[*strIndex] )
	{
		typeTag = 'W';
	}
	else
	{
		typeTag = 'G';
		if('o' == line[*strIndex])
		{
			toReturn -> outputFlag = 1;
			while( line[*strIndex] != ' ' )
			{
				*strIndex = *strIndex + 1;
			}
		}

		//toReturn -> gate_data = processGate(line, *strIndex, circuit, toReturn);
		gate_data = processGate(line, *strIndex, toReturn);
		recursiveEncryptionTree();
	}
	gateList[*gateIndex] = *this;
	*gateIndex ++;
}


void gateOrWire::recursiveEncryptionTree()
{
	unsigned char *keyList[gate_data -> numInputs];
	struct outputEncRow *tempRow;
	int i, j, k, tempBit;

	for(i = 0; i < gate_data -> outputTableSize; i ++)
	{
		tempRow = gate_data -> outputTreeEnc;
		for(j = 0; j < gate_data -> numInputs; j ++)
		{
			tempBit = (i >> j) & 1;
			tempRow = tempRow -> keyChoice[tempBit];

			if(0 == tempBit)
				keyList[j] = gate_data -> inputKeySet[j] -> key0;
			else if(1 == tempBit)
				keyList[j] = gate_data -> inputKeySet[j] -> key1;
		}

		if(0 == tempRow -> outputValue)		
			tempRow -> outputEncValue = encryptMultiple(keyList, gate_data -> numInputs, outputGarbleKeys -> key0);
		else if(1 == tempRow -> outputValue)
			tempRow -> outputEncValue = encryptMultiple(keyList, gate_data -> numInputs, outputGarbleKeys -> key1);
	}
}


void gateOrWire::decryptionTree(gateOrWire *inputCircuit)
{
	unsigned char *keyList[gate_data -> numInputs];
	struct outputEncRow *tempRow;
	int i, j, k, tempBit, tempIndex;

	tempRow = gate_data -> outputTreeEnc;
	for(j = 0; j < gate_data -> numInputs; j ++)
	{
		tempIndex = gate_data -> inputIDs[j];
		tempBit = inputCircuit[tempIndex].wireValue;
		tempRow = tempRow -> keyChoice[tempBit];

		if(0 == tempBit)
			keyList[j] = gate_data -> inputKeySet[j] -> key0;
		else if(1 == tempBit)
			keyList[j] = gate_data -> inputKeySet[j] -> key1;
	}

	wireValue = tempRow -> outputValue;
	if(0 == tempRow -> outputValue)
		inputCircuit[i].wireEncValue = decryptMultiple(keyList, gate_data -> numInputs, tempRow -> outputEncValue);
	else if(1 == tempRow -> outputValue)
		inputCircuit[i].wireEncValue = decryptMultiple(keyList, gate_data -> numInputs, tempRow -> outputEncValue);
}


void gateOrWire::generateGarbleKeyPair()
{
	struct bitsGarbleKeys *toReturn = (struct bitsGarbleKeys *)calloc(1, sizeof(struct bitsGarbleKeys));

	RAND_bytes(toReturn -> key0, 16);
	RAND_bytes(toReturn -> key1, 16);

	outputGarbleKeys = toReturn;
}


void gateOrWire::getInputGarbleKeys(gateOrWire *inputCircuit, int *inputIDs)
{
	struct bitsGarbleKeys **inputKeys = (struct bitsGarbleKeys **) calloc(gate_data -> numInputs, sizeof(struct bitsGarbleKeys*));
	int i, gid;

	for(i = 0; i < gate_data -> numInputs; i ++)
	{
		gid = *(inputIDs + i);
		inputKeys[i] = inputCircuit[gid].outputGarbleKeys;
	}

	gate_data -> inputKeySet = inputKeys;
}

void printGate(struct gate *input)
{
	int i;

	printf("NumInputs = %d\n", input -> numInputs);

	printf("[ %d", *((input -> inputIDs)) );
	for(i = 1; i < input -> numInputs; i ++)
	{
		printf(", %d", *((input -> inputIDs) + i) );
	}
	printf(" ]\n");
}


void gateOrWire::printGateOrWire()
{
	if( 'W' == typeTag )
	{
		printf("Wire ID  = %d\n", G_ID);
	}
	else if( 'G' == typeTag )
	{
		printf("Gate ID  = %d\n", G_ID);
		printGate( gate_data );
	}

	printf("Value = %d\n", wireValue);
}
