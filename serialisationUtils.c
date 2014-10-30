#ifndef SERIALISATION_UTILS
#define SERIALISATION_UTILS


int getSerialiseSize(struct gateOrWire *inputGW, int outputTableSize, int numInputs)
{
	int serialisedSize = 7;

	if(NULL != inputGW -> gatePayload)
		serialisedSize += (32 * outputTableSize) + (4 * numInputs);

	if(0x0F == inputGW -> outputWire -> wireMask)
		serialisedSize ++;
	else if (0xFF == inputGW -> outputWire -> wireOwner)
		serialisedSize += 32;
	
	if(0xF0 == inputGW -> outputWire -> wireMask)
	{
		serialisedSize ++;
		if(0xFF == inputGW -> outputWire -> wireOwner)
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


int serialiseInputWire(struct gateOrWire *inputGW, unsigned char *toReturn, int offset)
{
	int i, j, curIndex = offset;

	if(0x00 == inputGW -> outputWire -> wireOwner)
	{
		toReturn[curIndex] = inputGW -> outputWire -> wirePerm;
		curIndex ++;
	}
	else if(0xFF == inputGW -> outputWire -> wireOwner)
	{
		toReturn[curIndex] = inputGW -> outputWire -> wirePermedValue;
		memcpy(toReturn + 1 + curIndex, inputGW -> outputWire -> wireOutputKey, 16);
		curIndex += 17;
	}

	return curIndex;
}


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
		memcpy(toReturn + j, inputGate -> encOutputTable + i, 32);
	}
	curIndex = curIndex + (4 * inputGate -> numInputs)
						+ (32 * inputGate -> outputTableSize);

	return curIndex;
}


unsigned char *serialiseGateOrWire(struct gateOrWire *inputGW, int *outputLength)
{
	unsigned char *toReturn;
	int i, j, outputTableSize = 0, numInputs = 0;
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





int deserialiseInputWire(struct gateOrWire *outputGW, unsigned char *serialGW, int offset)
{
	int i, j, curIndex = offset;

	if(0x00 == outputGW -> outputWire -> wireOwner)
	{
		outputGW -> outputWire -> wirePerm = serialGW[curIndex];
		curIndex ++;
	}
	else if(0xFF == outputGW -> outputWire -> wireOwner)
	{
		outputGW -> outputWire -> wirePermedValue = serialGW[curIndex]; 
		memcpy(outputGW -> outputWire -> wireOutputKey, serialGW + 1 + curIndex, 16);
		curIndex += 17;
	}

	return curIndex;
}


int deserialiseGate(struct gate *outputGate, unsigned char *toReturn, int offset)
{
	int i, j, curIndex = offset;

	outputGate -> numInputs = toReturn[curIndex++];
	memcpy(&outputGate -> outputTableSize, toReturn + curIndex, 2);
	curIndex += 2;

	outputGate -> inputIDs = (int*) calloc(outputGate -> numInputs, sizeof(int));
	for(i = 0; i < outputGate -> numInputs; i ++)
	{
		j = curIndex + 4 * i;
		memcpy(outputGate -> inputIDs[i], toReturn + j, 4);
	}
	curIndex += (4 * outputGate -> numInputs);

	outputGate = (unsigned char **) calloc(outputGate -> outputTableSize, sizeof(unsigned char*));
	for(i = 0; i < outputGate -> outputTableSize; i ++)
	{
		j = curIndex + 32 * i;
		outputGate -> encOutputTable[i] = (unsigned char*) calloc(32, sizeof(unsigned char));
		memcpy(outputGate -> encOutputTable[i], toReturn + j, 32);
	}
	curIndex += (32 * outputGate -> outputTableSize);

	return curIndex;
}


// Function to deserialise an unsigned chars array to a gateOrWire struct.
struct gateOrWire *deserialiseGateOrWire(unsigned char *serialGW)
{
	struct gateOrWire *toReturn;
	struct gate *tempGate = NULL;
	int i, j, k, curIndex = 7;
	int outputTableSize = 0, numInputs = 0;

	// Calloc space for gateOrWire, and for the outputWire.
	toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));
	toReturn -> outputWire = (struct wire*) calloc(1, sizeof(struct wire));

	// Copy G_ID, wireMask, wireOwner from string. Initialise wirePerm to zero.
	memcpy(&(toReturn -> G_ID), serialGW, 4);
	toReturn -> outputWire -> wireMask = serialGW[5];
	toReturn -> outputWire -> wireOwner = serialGW[6];
	toReturn -> outputWire -> wirePerm = 0;
	
	// Calloc space for the wireOutputKey. If wire owner by Builder copy in 
	toReturn -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));

	if(0xF0 == toReturn -> outputWire -> wireMask)
	{
		curIndex = deserialiseInputWire(toReturn, serialGW, curIndex);
	}
	else 
	{
		if(0xFF == toReturn -> gatePayload)
		{
			tempGate = (struct gate*) calloc(1, sizeof(struct gate*));
			curIndex = deserialiseGate(tempGate, serialGW, curIndex);
		}

		if(0x0F == toReturn -> outputWire -> wireMask)
		{
			toReturn -> outputWire -> wirePerm = serialGW[curIndex];
			curIndex ++;
		}
	}
	// Assign the tempGate to the gatePayload of the gateOrWire to return.
	toReturn -> gatePayload = tempGate;

	return toReturn;
}


#endif