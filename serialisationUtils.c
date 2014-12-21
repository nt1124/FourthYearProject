#ifndef SERIALISATION_UTILS
#define SERIALISATION_UTILS



// Get the size of the character buffer we need to store the input gateOrWire in.
int getSerialiseSize(struct gateOrWire *inputGW, int outputTableSize, int numInputs)
{
	int serialisedSize = 8;

	if(NULL != inputGW -> gatePayload)
		serialisedSize += (32 * outputTableSize) + (4 * numInputs);

	if(0x0F == inputGW -> outputWire -> wireMask)
		serialisedSize ++;
	else if (0xFF == inputGW -> outputWire -> wireOwner)
		serialisedSize += 32;
	
	if(0xF0 == inputGW -> outputWire -> wireMask)
	{
		serialisedSize ++;
		serialisedSize += 16;
		if(0x00 == inputGW -> outputWire -> wireOwner)
		{
			serialisedSize += 16;
		}
	}
	else 
	{
		if(NULL != inputGW -> gatePayload)
		{
			serialisedSize += (3 + 4 * numInputs + 32 * outputTableSize);
		}

		if(0x0F == inputGW -> outputWire -> wireMask)
		{
			serialisedSize ++;
		}
	}

	return serialisedSize;
}


// Does the actual serialisation of an input wire into a buffer of uchars. Returns length of
// the buffer.
int serialiseInputWire(struct gateOrWire *inputGW, unsigned char *toReturn, int offset)
{
	int curIndex = offset;

	if(0x00 == inputGW -> outputWire -> wireOwner)
	{
		toReturn[curIndex ++] = inputGW -> outputWire -> wirePerm;
		memcpy(toReturn + curIndex, inputGW -> outputWire -> outputGarbleKeys -> key0, 16);
		curIndex += 16;
		memcpy(toReturn + curIndex, inputGW -> outputWire -> outputGarbleKeys -> key1, 16);
		curIndex += 16;
	}
	else if(0xFF == inputGW -> outputWire -> wireOwner)
	{
		toReturn[curIndex ++] = inputGW -> outputWire -> wirePermedValue;
		memcpy(toReturn + curIndex, inputGW -> outputWire -> wireOutputKey, 16);
		curIndex += 16;
	}

	return curIndex;
}


// Handles the serialisation of the gate part of a gateOrWire. Returns length of buffer.
int serialiseGate(struct gate *inputGate, unsigned char *toReturn, int offset)
{
	int i, j, curIndex = offset;

	toReturn[curIndex++] = inputGate -> numInputs;
	
	memcpy(toReturn + curIndex, &inputGate -> outputTableSize, 2);
	curIndex += 2;

	for(i = 0; i < inputGate -> numInputs; i ++)
	{
		j = curIndex + 4 * i;
		memcpy(toReturn + j, inputGate -> inputIDs + i, 4);
	}

	for(i = 0; i < inputGate -> outputTableSize; i ++)
	{
		j = curIndex + 4 * inputGate -> numInputs + 32 * i;
		memcpy(toReturn + j, inputGate -> encOutputTable[i], 32);
	}
	curIndex = curIndex + (4 * inputGate -> numInputs)
						+ (32 * inputGate -> outputTableSize);

	return curIndex;
}


// Handles the serialisation of a gateOrWire. Returns length of buffer.
unsigned char *serialiseGateOrWire(struct gateOrWire *inputGW, int *outputLength)
{
	unsigned char *toReturn;
	int outputTableSize = 0, numInputs = 0;
	int serialisedSize = 0, curIndex = 7;

	if(NULL != inputGW -> gatePayload)
	{
		outputTableSize = inputGW -> gatePayload -> outputTableSize;
		numInputs = inputGW -> gatePayload -> numInputs;
	}

	serialisedSize = getSerialiseSize(inputGW, outputTableSize, numInputs);

	toReturn = (unsigned char*) calloc(serialisedSize, sizeof(unsigned char));

	memcpy(toReturn, &(inputGW -> G_ID), 4);
	if(NULL == inputGW -> gatePayload)
		toReturn[4] = 0x00;
	else
		toReturn[4] = 0xFF;
	toReturn[5] = inputGW -> outputWire -> wireMask;
	toReturn[6] = inputGW -> outputWire -> wireOwner;

	if(0xF0 == inputGW -> outputWire -> wireMask)
	{
		curIndex = serialiseInputWire(inputGW, toReturn, curIndex);
	}
	else 
	{
		if(NULL != inputGW -> gatePayload)
		{
			curIndex = serialiseGate(inputGW -> gatePayload, toReturn, curIndex);
		}

		if(0x0F == inputGW -> outputWire -> wireMask)
		{
			toReturn[curIndex] = inputGW -> outputWire -> wirePerm;
			curIndex ++;
		}
	}

	*outputLength = serialisedSize;

	return toReturn;
}




// Deserialise a uchar buffer into an input gateOrWire
int deserialiseInputWire(struct gateOrWire *outputGW, unsigned char *serialGW, int offset)
{
	int curIndex = offset;

	if(0x00 == outputGW -> outputWire -> wireOwner)
	{
		outputGW -> outputWire -> wirePerm = serialGW[curIndex ++];
		outputGW -> outputWire -> outputGarbleKeys = (struct bitsGarbleKeys*) calloc(1, sizeof(struct bitsGarbleKeys));
		
		outputGW -> outputWire -> outputGarbleKeys -> key0 = (unsigned char *) calloc(16, sizeof(unsigned char));
		memcpy(outputGW -> outputWire -> outputGarbleKeys -> key0, serialGW + curIndex, 16);
		curIndex += 16;

		outputGW -> outputWire -> outputGarbleKeys -> key1 = (unsigned char *) calloc(16, sizeof(unsigned char));
		memcpy(outputGW -> outputWire -> outputGarbleKeys -> key1, serialGW + curIndex, 16);
		curIndex += 16;
	}
	else if(0xFF == outputGW -> outputWire -> wireOwner)
	{
		outputGW -> outputWire -> wirePermedValue = serialGW[curIndex ++];
		memcpy(outputGW -> outputWire -> wireOutputKey, serialGW + curIndex, 16);
		curIndex += 16;
	}

	return curIndex;
}


// Deserialise a section of a (offset) uchar buffer into a gate.
int deserialiseGate(struct gate **outputGate, unsigned char *serialGW, int offset)
{
	int i, j, curIndex = offset;
	struct gate *tempGate = (struct gate*) calloc(1, sizeof(struct gate));

	// Get the number of input and the size of output table.
	tempGate -> numInputs = serialGW[curIndex++];
	memcpy(&tempGate -> outputTableSize, serialGW + curIndex, 2);
	curIndex += 2;

	// Calloc space for inputIDs table, then get inputIDs from serialisation.
	tempGate -> inputIDs = (int*) calloc(tempGate -> numInputs, sizeof(int));
	for(i = 0; i < tempGate -> numInputs; i ++)
	{
		j = curIndex + 4 * i;
		memcpy(&tempGate -> inputIDs[i], serialGW + j, 4);
	}
	curIndex += (4 * tempGate -> numInputs);

	// Export the output gate, then NULL tempGate for the heck of it.
	*outputGate = tempGate;
	tempGate = NULL;

	// Return the index in the serialisation string at end of reading.
	return curIndex;
}


// Function to deserialise an unsigned chars array to a gateOrWire struct.
struct gateOrWire *deserialiseGateOrWire(unsigned char *serialGW)
{
	unsigned char **tempEncOut;
	struct gateOrWire *toReturn;
	struct gate *tempGate = NULL;
	int i, curIndex = 7;

	// Calloc space for gateOrWire, and for the outputWire.
	toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));
	toReturn -> outputWire = (struct wire*) calloc(1, sizeof(struct wire));
	toReturn -> gatePayload = NULL;

	// Copy G_ID, wireMask, wireOwner from string. Initialise wirePerm to zero.
	memcpy(&(toReturn -> G_ID), serialGW, 4);
	toReturn -> outputWire -> wireMask = serialGW[5];
	toReturn -> outputWire -> wireOwner = serialGW[6];
	toReturn -> outputWire -> wirePerm = 0;
	
	// Calloc space for the wireOutputKey. 
	toReturn -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));

	// If the wire is an input wire.
	if(0xF0 == toReturn -> outputWire -> wireMask)
	{
		curIndex = deserialiseInputWire(toReturn, serialGW, curIndex);
	}
	else 
	{
		if(0xFF == serialGW[4])
		{
			curIndex = deserialiseGate(&tempGate, serialGW, curIndex);

			// Calloc space for the encrypted output table, segfaults if this is done inside deserialiseGate.
			tempEncOut = (unsigned char **) calloc(tempGate -> outputTableSize, sizeof(unsigned char*));
			for(i = 0; i < tempGate -> outputTableSize; i ++)
			{
				tempEncOut[i] = (unsigned char*) calloc(32, sizeof(unsigned char));
			}
			tempGate -> encOutputTable = tempEncOut;
			curIndex += (32 * tempGate -> outputTableSize);
		}

		if(0x0F == toReturn -> outputWire -> wireMask)
		{
			toReturn -> outputWire -> wirePerm = serialGW[curIndex];
			curIndex ++;
		}
	}

	// Assign the tempGate to the gatePayload of the gateOrWire to return.
	toReturn -> gatePayload = tempGate;
	tempGate = NULL;

	return toReturn;
}


// Runs tests to make sure correctness of serialisation and deserialisation.
void testSerialisation(struct gateOrWire *inputGW)
{
	unsigned char *serialGW;
	int serialLength = 0, i, j;
	struct gateOrWire *outputGW;
	unsigned char *temp;


	serialGW = serialiseGateOrWire(inputGW, &serialLength);
	outputGW = deserialiseGateOrWire(serialGW);
	free(serialGW);


	printf("\nBefore serialisation\n");
	printf("G_ID				=  %d\n", inputGW -> G_ID);
	printf("Wire Mask			=  %02X\n", inputGW -> outputWire -> wireMask);
	printf("Wire Owner			=  %02X\n", inputGW -> outputWire -> wireOwner);
	printf("Wire Perm			=  %02X\n", inputGW -> outputWire -> wirePerm);
	printf("Out key 			=  ");
	for(i = 0; i < 16; i ++)
		printf("%02X", inputGW -> outputWire -> wireOutputKey[i]);
	printf("\n");

	if(0x00 == inputGW -> outputWire -> wireOwner &&
	   0xF0 == inputGW -> outputWire -> wireMask)
	{
		printf("key0 				=  ");
		for(i = 0; i < 16; i ++)
			printf("%02X", inputGW -> outputWire -> outputGarbleKeys -> key0[i]);
		printf("\n");

		printf("key1 				=  ");
		for(i = 0; i < 16; i ++)
			printf("%02X", inputGW -> outputWire -> outputGarbleKeys -> key1[i]);
		printf("\n");
	}

	if(NULL != inputGW -> gatePayload)
	{
		printf("numInputs			=  %d\n", inputGW -> gatePayload -> numInputs);
		printf("outputTableSize 		=  %d\n", inputGW -> gatePayload -> outputTableSize);

		for(i = 0; i < inputGW -> gatePayload -> numInputs; i ++)
			printf("inputIDs[%d]			=  %d\n", i, inputGW -> gatePayload -> inputIDs[i]);

		temp = (unsigned char*) calloc(32, sizeof(unsigned char));
		for(i = 0; i < inputGW -> gatePayload -> outputTableSize; i ++)
		{
			printf("encOutputTable[%d]		=  ", i);
			memcpy(temp, inputGW -> gatePayload -> encOutputTable[i], 32);
			for(j = 0; j < 32; j ++)
			{
				printf("%02X", temp[j]);
			}
			printf("\n");
		}
		free(temp);
	}


	printf("\nSerialised and back again\n");
	printf("G_ID				=  %d\n", outputGW -> G_ID);
	printf("Wire Mask			=  %02X\n", outputGW -> outputWire -> wireMask);
	printf("Wire Owner			=  %02X\n", outputGW -> outputWire -> wireOwner);
	printf("Wire Perm			=  %02X\n", outputGW -> outputWire -> wirePerm);
	printf("Out key 			=  ");
	for(i = 0; i < 16; i ++)
		printf("%02X", outputGW -> outputWire -> wireOutputKey[i]);
	printf("\n");

	if(0x00 == outputGW -> outputWire -> wireOwner &&
	   0xF0 == outputGW -> outputWire -> wireMask)
	{
		printf("key0 				=  ");
		for(i = 0; i < 16; i ++)
			printf("%02X", outputGW -> outputWire -> outputGarbleKeys -> key0[i]);
		printf("\n");

		printf("key1 				=  ");
		for(i = 0; i < 16; i ++)
			printf("%02X", outputGW -> outputWire -> outputGarbleKeys -> key1[i]);
		printf("\n");
	}

	if(NULL != outputGW -> gatePayload)
	{
		printf("numInputs			=  %d\n", outputGW -> gatePayload -> numInputs);
		printf("outputTableSize 		=  %d\n", outputGW -> gatePayload -> outputTableSize);

		for(i = 0; i < outputGW -> gatePayload -> numInputs; i ++)
			printf("inputIDs[%d]			=  %d\n", i, outputGW -> gatePayload -> inputIDs[i]);

		temp = (unsigned char*) calloc(32, sizeof(unsigned char));
		for(i = 0; i < outputGW -> gatePayload -> outputTableSize; i ++)
		{
			printf("encOutputTable[%d]		=  ", i);
			memcpy(temp, outputGW -> gatePayload -> encOutputTable[i], 32);
			for(j = 0; j < 32; j ++)
			{
				printf("%02X", temp[j]);
			}
			printf("\n");
		}
		free(temp);
	}

	freeGateOrWire(outputGW);
}



#endif