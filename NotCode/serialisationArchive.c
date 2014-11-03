// IGNORE FOR NOW



struct wire *altDeserialiseInputWire(unsigned char *serialGW, int *offset)
{
	struct wire *outputWire= (struct wire*) calloc(1, sizeof(struct wire));
	int curIndex = *offset;

	outputWire -> wireMask = serialGW[5];
	outputWire -> wireOwner = serialGW[6];
	outputWire -> wirePerm = 0;
	outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));

	if(0xF0 == outputWire -> wireMask)
	{
		if(0x00 == outputWire -> wireOwner)
		{
			outputWire -> wirePerm = serialGW[curIndex ++];
			outputWire -> outputGarbleKeys = (struct bitsGarbleKeys*) calloc(1, sizeof(struct bitsGarbleKeys));
			
			outputWire -> outputGarbleKeys -> key0 = (unsigned char *) calloc(16, sizeof(unsigned char));
			memcpy(outputWire -> outputGarbleKeys -> key0, serialGW + curIndex, 16);
			curIndex += 16;

			outputWire -> outputGarbleKeys -> key1 = (unsigned char *) calloc(16, sizeof(unsigned char));
			memcpy(outputWire -> outputGarbleKeys -> key1, serialGW + curIndex, 16);
			curIndex += 16;
		}
		else if(0xFF == outputWire -> wireOwner)
		{
			outputWire -> wirePermedValue = serialGW[curIndex ++];
			memcpy(outputWire -> wireOutputKey, serialGW + curIndex, 16);
			curIndex += 16;
		}
	}

	*offset = curIndex;
	return outputWire;
}


struct gate *altDeserialiseGate(unsigned char *serialGW, int *offset)
{
	struct gate *outputGate = (struct gate*) calloc(1, sizeof(struct gate*));
	int i, j, curIndex = *offset;

	outputGate -> numInputs = serialGW[curIndex++];
	memcpy(&outputGate -> outputTableSize, serialGW + curIndex, 2);
	curIndex += 2;

	outputGate -> inputIDs = (int*) calloc(outputGate -> numInputs, sizeof(int));
	for(i = 0; i < outputGate -> numInputs; i ++)
	{
		j = curIndex + 4 * i;
		memcpy(&outputGate -> inputIDs[i], serialGW + j, 4);
	}
	curIndex += (4 * outputGate -> numInputs);

	*offset = curIndex;
	return outputGate;
}


// Function to deserialise an unsigned chars array to a gateOrWire struct.
struct gateOrWire *altDeserialiseGateOrWire(unsigned char *serialGW)
{
	struct gateOrWire *toReturn;
	unsigned char **tempEncOut;
	int curIndex = 7;
	int i, j, k;

	// Calloc space for gateOrWire, and for the outputWire.
	toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));
	toReturn -> outputWire = NULL;
	toReturn -> gatePayload = NULL;

	// Copy G_ID, wireMask, wireOwner from string. Initialise wirePerm to zero.
	memcpy(&(toReturn -> G_ID), serialGW, 4);

	toReturn -> outputWire = altDeserialiseInputWire(serialGW, &curIndex);

	if(0x0F == toReturn -> outputWire -> wireMask)
	{
		if(0xFF == serialGW[4])
		{
			toReturn -> gatePayload = altDeserialiseGate(serialGW, &curIndex);

			tempEncOut = (unsigned char **) calloc(toReturn -> gatePayload -> outputTableSize, sizeof(unsigned char*));
			for(i = 0; i < toReturn -> gatePayload -> outputTableSize; i ++)
			{
				j = curIndex + 32 * i;
				tempEncOut[i] = (unsigned char*) calloc(32, sizeof(unsigned char));
				for(k = 0; k < 32; k ++)
				{
					tempEncOut[i][k] = serialGW[j + k];
				}
			}
			toReturn -> gatePayload -> encOutputTable = tempEncOut;
			curIndex += (32 * toReturn -> gatePayload -> outputTableSize);
		}

		if(0x0F == toReturn -> outputWire -> wireMask)
		{
			toReturn -> outputWire -> wirePerm = serialGW[curIndex];
			curIndex ++;
		}
	}


	return toReturn;
}