#ifndef CIRCUIT_CHECKER_UTILS
#define CIRCUIT_CHECKER_UTILS


// Decrypt the gate using it's inputs, eventually should only be used for the non-XOR gates.
void fullyDecryptGate(struct gateOrWire *curGate, struct gateOrWire **inputCircuit)
{
	int numInputs = curGate -> gatePayload -> numInputs;
	int i, j, tempIndex, outputIndex = 0, index;
	unsigned char *keyList[numInputs], *toReturn, tempBit;
	unsigned char *tempRow;
	struct wire *tempWire;


	curGate -> outputWire -> outputGarbleKeys = (struct bitsGarbleKeys*) calloc(1, sizeof(struct bitsGarbleKeys));
	curGate -> outputWire -> outputGarbleKeys -> key0 = NULL;
	curGate -> outputWire -> outputGarbleKeys -> key1 = NULL;
	curGate -> gatePayload -> rawOutputTable = (int*) calloc(curGate -> gatePayload -> outputTableSize, sizeof(int));

	for(i = 0; i < curGate -> gatePayload -> outputTableSize; i ++)
	{
		index = 0;
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

		// Get the row of the output table indexed by the values of our input wires.
		tempRow = curGate -> gatePayload -> encOutputTable[outputIndex];

		// Then decrypt this row using the key list, copy first 16 bits into the output key field.
		toReturn = decryptMultipleKeys(keyList, numInputs, tempRow, 2);

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


/*
int numInputs = curGate -> gatePayload -> numInputs;
int i, j, tempIndex, outputIndex = 0;
unsigned char *toReturn = (unsigned char *) calloc(16, sizeof(unsigned char));
unsigned char outputBit = 0x00;

// For the all the input bits 
for(i = 0; i < numInputs; i ++)
{
	// Get the index of the i^th input wire, then get that wires permutated output value.
	tempIndex = curGate -> gatePayload -> inputIDs[numInputs - i - 1];
	outputBit ^= inputCircuit[tempIndex] -> outputWire -> wirePermedValue;

	for(j = 0; j < 16; j ++)
	{
		toReturn[j] ^= inputCircuit[tempIndex] -> outputWire -> wireOutputKey[j];
	}
}

// Get the decrypted permutated wire value.
curGate -> outputWire -> wirePermedValue = outputBit;

// Housekeeping.
memcpy(curGate -> outputWire -> wireOutputKey, toReturn, 16);
free(toReturn);
*/


#endif