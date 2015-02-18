#ifndef CIRCUIT_CHECKER_UTILS
#define CIRCUIT_CHECKER_UTILS

#include "../fileUtils/inputFileUtils.h"


int checkForSignalBit(unsigned char *inputToCheck)
{
	unsigned char *temp = (unsigned char*) calloc(16, sizeof(unsigned char));

	if(memcmp(temp, inputToCheck + 16, 16) == 0)
	{
		free(temp);
		return 0;
	}
	
	temp[0] = 0x01;

	if(memcmp(temp, inputToCheck + 16, 16) == 0)
	{
		free(temp);
		return 0;
	}

	free(temp);
	return 1;
}


// Decrypt the gate using it's inputs, eventually should only be used for the non-XOR gates.
void fullyDecryptGate(struct gateOrWire *curGate, struct gateOrWire **inputCircuit)
{
	int numInputs = curGate -> gatePayload -> numInputs;
	int i, j, tempIndex, outputIndex = 0;
	unsigned char *keyList[numInputs], *toReturn, tempBit;
	unsigned char *tempRow;
	struct wire *tempWire;

	int k;


	curGate -> outputWire -> outputGarbleKeys = (struct bitsGarbleKeys*) calloc(1, sizeof(struct bitsGarbleKeys));
	curGate -> outputWire -> outputGarbleKeys -> key0 = NULL;
	curGate -> outputWire -> outputGarbleKeys -> key1 = NULL;
	curGate -> gatePayload -> rawOutputTable = (int*) calloc(curGate -> gatePayload -> outputTableSize, sizeof(int));

	for(i = 0; i < curGate -> gatePayload -> outputTableSize; i ++)
	{
		outputIndex = 0;
		// For the all the input bits 
		for(j = 0; j < numInputs; j ++)
		{
			// Get the index of the j^th input wire, then get that wires permutated output value.
			tempIndex = curGate -> gatePayload -> inputIDs[numInputs - j - 1];
			tempWire = inputCircuit[tempIndex] -> outputWire;

			// Get the value of this wire for this bit of this input.
			tempBit = (i >> j) & 0x01;

			// Copy into our key list
			keyList[j] = (unsigned char*) calloc(16, sizeof(unsigned char));
			if(0 == tempBit)
				memcpy(keyList[j], tempWire -> outputGarbleKeys -> key0, 16);
			else if(1 == tempBit)
				memcpy(keyList[j], tempWire -> outputGarbleKeys -> key1, 16);
		}

		j = -1;
		do 
		{
			j ++;
			// Get the row of the output table indexed by the values of our input wires.
			tempRow = curGate -> gatePayload -> encOutputTable[j];

			// Then decrypt this row using the key list, copy first 16 bits into the output key field.
			toReturn = decryptMultipleKeys(keyList, numInputs, tempRow, 2);
		}
		while(j < curGate -> gatePayload -> outputTableSize && checkForSignalBit(toReturn));

		// printf(">>>   %X  %X  %X  %X\n", i, j, i ^ j, toReturn[16]);

		// Get the decrypted permutated wire value.
		curGate -> gatePayload -> rawOutputTable[i] = (int)toReturn[16];

		if(NULL == curGate -> outputWire -> outputGarbleKeys -> key0 && 0x00 == toReturn[16])
		{
			curGate -> outputWire -> outputGarbleKeys -> key0 = (unsigned char *) calloc(16, sizeof(unsigned char));
			memcpy(curGate -> outputWire -> outputGarbleKeys -> key0, toReturn, 16);
		}
		else if(NULL == curGate -> outputWire -> outputGarbleKeys -> key1 && 0x01 == toReturn[16])
		{
			curGate -> outputWire -> outputGarbleKeys -> key1 = (unsigned char *) calloc(16, sizeof(unsigned char));
			memcpy(curGate -> outputWire -> outputGarbleKeys -> key1, toReturn, 16);
		}

		// Housekeeping.
		for(j = 0; j < numInputs; j ++)
		{
			free(keyList[j]);
		}
		free(toReturn);
	}

}



// Decrypt the gate using it's inputs, eventually should only be used for the non-XOR gates.
void fullyXOR_DecryptGate(struct gateOrWire *curGate, struct gateOrWire **inputCircuit)
{
	int numInputs = curGate -> gatePayload -> numInputs;
	int i, j, k, tempIndex, outputIndex = 0, index;
	unsigned char *toReturn, *tempRow, outputBit = 0x00, tempBit;
	struct wire *tempWire;


	curGate -> outputWire -> outputGarbleKeys = (struct bitsGarbleKeys*) calloc(1, sizeof(struct bitsGarbleKeys));
	curGate -> outputWire -> outputGarbleKeys -> key0 = NULL;
	curGate -> outputWire -> outputGarbleKeys -> key1 = NULL;
	curGate -> gatePayload -> rawOutputTable = (int*) calloc(curGate -> gatePayload -> outputTableSize, sizeof(int));

	for(i = 0; i < curGate -> gatePayload -> outputTableSize; i ++)
	{
		toReturn = (unsigned char *) calloc(16, sizeof(unsigned char));
		index = 0;
		// For the all the input bits 
		for(j = 0; j < numInputs; j ++)
		{
			tempIndex = curGate -> gatePayload -> inputIDs[numInputs - i - 1];
			outputBit ^= inputCircuit[tempIndex] -> outputWire -> wirePermedValue;
			tempBit = (i >> j) & 0x01;


			if(0x00 == tempBit)
			{
				for(k = 0; k < 16; k ++)
				{
					toReturn[k] ^= inputCircuit[tempIndex] -> outputWire -> outputGarbleKeys -> key0[k];
				}
			}
			else
			{
				for(k = 0; k < 16; k ++)
				{
					toReturn[k] ^= inputCircuit[tempIndex] -> outputWire -> outputGarbleKeys -> key1[k];
				}
			}
		}


		// Get the decrypted permutated wire value.
		curGate -> gatePayload -> rawOutputTable[i] = (int)toReturn[16];

		if(NULL == curGate -> outputWire -> outputGarbleKeys -> key0 && 0x00 == toReturn[16])
		{
			curGate -> outputWire -> outputGarbleKeys -> key0 = (unsigned char *) calloc(16, sizeof(unsigned char));
			memcpy(curGate -> outputWire -> outputGarbleKeys -> key0, toReturn, 16);
		}
		else if(NULL == curGate -> outputWire -> outputGarbleKeys -> key1 && 0x01 == toReturn[16])
		{
			curGate -> outputWire -> outputGarbleKeys -> key0 = (unsigned char *) calloc(16, sizeof(unsigned char));
			memcpy(curGate -> outputWire -> outputGarbleKeys -> key1, toReturn, 16);
		}

		free(toReturn);
	}
}


void fullyEvaluateGate(struct gateOrWire *curGate, struct gateOrWire **inputCircuit)
{
	if( 0xF0 != (0xF0 & curGate -> outputWire -> wireMask) )
	{
		fullyDecryptGate(curGate, inputCircuit);
	}
	else
	{
		fullyXOR_DecryptGate(curGate, inputCircuit);
	}
}




void fullyDecryptCircuit(struct Circuit *inputCircuit)
{
	int i, gateID, j;

	for(i = 0; i < inputCircuit -> numGates; i ++)
	{
		gateID = inputCircuit -> execOrder[i];

		printf("<%d><%d>   -   %02X\n", i, gateID, inputCircuit -> gates[gateID] -> outputWire -> wirePerm & 0x01);

		if(NULL != inputCircuit -> gates[gateID] -> gatePayload)
		{
			fflush(stdout);
			fullyEvaluateGate(inputCircuit -> gates[gateID], inputCircuit -> gates);
		}
	}
}




int compareGate(struct gate *gateA, struct gate *gateB)
{
	int output = 0, i, j;

	output |= (0x01 ^ (gateA -> numInputs == gateB -> numInputs));
	output |= (0x01 ^ (gateA -> outputTableSize == gateB -> outputTableSize));

	for(i = 0; i < gateA -> numInputs; i ++)
	{
		output |= (0x01 ^ (gateA -> inputIDs[i] == gateB -> inputIDs[i]));
	}

	
	for(i = 0; i < gateA -> outputTableSize; i ++)
	{
		output |= memcmp(gateA -> encOutputTable[i], gateB -> encOutputTable[i], 32);
	}
	

	return output;
}

int compareGateOrWirePair(struct gateOrWire *gateA, struct gateOrWire *gateB)
{
	int output = 0, i;

	output |= (0x01 ^ (gateA -> G_ID == gateB -> G_ID));

	if(NULL != gateA -> gatePayload && NULL != gateB -> gatePayload)
		output |= compareGate(gateA -> gatePayload, gateB -> gatePayload);
	else
		output |= (0x01 ^ (gateA -> gatePayload == gateB -> gatePayload));

	output |= memcmp(gateA -> outputWire -> outputGarbleKeys -> key0, gateB -> outputWire -> outputGarbleKeys -> key0, 16);
	output |= memcmp(gateA -> outputWire -> outputGarbleKeys -> key1, gateB -> outputWire -> outputGarbleKeys -> key1, 16);

	return output;
}




void testCircuitDecryption(char *circuitFilepath, char *inputFilepath_B, char *inputFilepath_E)
{
	struct RawCircuit *rawInputCircuit = readInCircuit_Raw(circuitFilepath);
	struct Circuit *garbledCircuit1, *garbledCircuit2;
	struct idAndValue *startOfInputChain, *start;
	unsigned char *seedBytes = generateRandBytes(sizeof(unsigned int), sizeof(unsigned int));
	unsigned int seedLong = 0;
	int i, temp = 0, tempTemp;

	memcpy(&seedLong, seedBytes, sizeof(unsigned int));


	garbledCircuit1 = readInCircuit_FromRaw_Seeded(rawInputCircuit, seedLong);
	garbledCircuit2 = readInCircuit_FromRaw_Seeded(rawInputCircuit, seedLong);


	/*
	startOfInputChain = readInputDetailsFile_Alt(inputFilepath_B);
	start = startOfInputChain;
	setCircuitsInputs_Hardcode(start, garbledCircuit1, 0x00);
	start = startOfInputChain;
	setCircuitsInputs_Hardcode(start, garbledCircuit2, 0x00);
	free_idAndValueChain(startOfInputChain);


	startOfInputChain = readInputDetailsFile_Alt(inputFilepath_E);
	start = startOfInputChain;
	setCircuitsInputs_Hardcode(start, garbledCircuit1, 0x00);
	start = startOfInputChain;
	setCircuitsInputs_Hardcode(start, garbledCircuit2, 0x00);
	free_idAndValueChain(startOfInputChain);
	*/

	for(i = 0; i < rawInputCircuit -> numGates; i ++)
	{
		temp += compareGateOrWirePair(garbledCircuit1 -> gates[i], garbledCircuit2 -> gates[i]);
	}
	printf("Comparison yield... %d\n", temp);
}




#endif