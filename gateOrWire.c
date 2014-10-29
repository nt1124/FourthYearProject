void encWholeOutTable(struct gateOrWire *curGate, struct gateOrWire **circuit)
{
	int numInputs = curGate -> gatePayload -> numInputs;
	int i, j, k, tempBit, permedIndex, inputID;
	struct wire *inputWire;
	unsigned char *keyList[numInputs], *tempRow;
	unsigned char *toEncrypt0 = (unsigned char*) calloc(32, sizeof(unsigned char));
	unsigned char *toEncrypt1 = (unsigned char*) calloc(32, sizeof(unsigned char));
	
	memcpy( toEncrypt0, curGate -> outputWire -> outputGarbleKeys -> key0, 17);
	memcpy( toEncrypt1, curGate -> outputWire -> outputGarbleKeys -> key1, 17);

	for(i = 0; i < curGate -> gatePayload -> outputTableSize; i ++)
	{
		permedIndex = 0;
		for(j = 0; j < numInputs; j ++)
		{
			inputID = curGate -> gatePayload -> inputIDs[j];
			inputWire = circuit[inputID] -> outputWire;

			tempBit = (i >> j) & 0x01;
			permedIndex ^= (inputWire -> wirePerm & 0x01) << j;

			keyList[j] = (unsigned char*) calloc(16, sizeof(unsigned char));
			if(0 == tempBit)
				memcpy(keyList[j], inputWire -> outputGarbleKeys -> key0, 16);
			else if(1 == tempBit)
				memcpy(keyList[j], inputWire -> outputGarbleKeys -> key1, 16);
		}
		permedIndex = i ^ permedIndex;

		if(0 == curGate -> gatePayload -> rawOutputTable[i])
			tempRow = encryptMultipleKeys(keyList, numInputs, toEncrypt0, 2);
		else if(1 == curGate -> gatePayload -> rawOutputTable[i])
			tempRow = encryptMultipleKeys(keyList, numInputs, toEncrypt1, 2);

		memcpy(curGate -> gatePayload -> encOutputTable[permedIndex], tempRow, 32);
	}
}


void decryptGate(struct gateOrWire *curGate, struct gateOrWire **inputCircuit, int sockfd)
{
	int numInputs = curGate -> gatePayload -> numInputs;
	int i, j, tempBit, tempIndex, outputIndex = 0;
	unsigned char *keyList[numInputs], *toReturn;
	unsigned char *tempRow;

	for(j = 0; j < numInputs; j ++)
	{
		tempIndex = curGate -> gatePayload -> inputIDs[numInputs - j - 1];
		tempBit = inputCircuit[tempIndex] -> outputWire -> wirePermedValue;

		outputIndex <<= 1;
		outputIndex += tempBit;

		keyList[j] = (unsigned char*) calloc(16, sizeof(unsigned char));
		memcpy(keyList[j], inputCircuit[tempIndex] -> outputWire -> wireOutputKey, 16);
	}

	tempRow = curGate -> gatePayload -> encOutputTable[outputIndex];
	toReturn = decryptMultipleKeys(keyList, numInputs, tempRow, 2);
	curGate -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));
	memcpy(curGate -> outputWire -> wireOutputKey, toReturn, 16);

	curGate -> outputWire -> wirePermedValue = toReturn[16];
}


int getSerialiseSize(struct gateOrWire *inputGW, int outputTableSize, int numInputs)
{
	int serialisedSize = 43;

	if(NULL != inputGW -> gatePayload)
		serialisedSize += (32 * outputTableSize) + (4 * numInputs);

	if(0x0F == inputGW -> outputWire -> wireMask)
		serialisedSize ++;
	else if (0xFF == inputGW -> outputWire -> wireOwner)
		serialisedSize += 32;


	return serialisedSize;
}


unsigned char *serialiseGateOrWire(struct gateOrWire *inputGW, int *outputLength)
{
	unsigned char *toReturn;
	int i, j, outputTableSize = 0, numInputs = 0;
	int serialisedSize = 0;

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

	if( 0xFF == inputGW -> outputWire -> wireOwner )
	{
		toReturn[7] = inputGW -> outputWire -> wirePermedValue;
		memcpy(toReturn + 8, inputGW -> outputWire -> wireOutputKey, 32);
	}

	if(NULL != inputGW -> gatePayload)
	{
		toReturn[40] = inputGW -> gatePayload -> numInputs;
		memcpy(toReturn + 41, &outputTableSize, 2);
		for(i = 0; i < inputGW -> gatePayload -> numInputs; i ++)
		{
			j = 43 + 4 * i;
			memcpy(toReturn + j, inputGW -> gatePayload -> inputIDs + i, 4);
		}

		for(i = 0; i < outputTableSize; i ++)
		{
			j = 43 + 4 * numInputs + 32 * i;
			memcpy(toReturn + j, inputGW -> gatePayload -> encOutputTable + i, 32);
		}
	}

	if(0x0F == toReturn[5] || 0x00 == toReturn[6])
		toReturn[43 + 4 * numInputs + 32 * outputTableSize] = inputGW -> outputWire -> wirePerm;

	*outputLength = serialisedSize;

	return toReturn;
}


struct gateOrWire *deserialiseGateOrWire(unsigned char *serialGW)
{
	struct gateOrWire *toReturn;
	struct gate *tempGate = NULL;
	int i, j, k, outputTableSize = 0, numInputs = 0;

	toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));
	toReturn -> outputWire = (struct wire*) calloc(1, sizeof(struct wire));

	memcpy(&(toReturn -> G_ID), serialGW, 4);
	toReturn -> outputWire -> wireMask = serialGW[5];
	toReturn -> outputWire -> wireOwner = serialGW[6];
	toReturn -> outputWire -> wirePerm = 0;
	
	toReturn -> outputWire -> wireOutputKey = (unsigned char*) calloc(32, sizeof(unsigned char));
	if( 0xFF == toReturn -> outputWire -> wireOwner )
	{
		toReturn -> outputWire -> wirePermedValue = serialGW[7];
		memcpy(toReturn -> outputWire -> wireOutputKey, serialGW + 8, 32);
	}

	if(0xFF == serialGW[4])
	{
		tempGate = (struct gate*) calloc(1, sizeof(struct gate));
		tempGate -> numInputs = serialGW[40];
		memcpy(&(tempGate -> outputTableSize), serialGW + 41, 2);

			tempGate -> inputIDs = (int*) calloc(tempGate -> numInputs, sizeof(int));
		for(i = 0; i < tempGate -> numInputs; i ++)
		{
			j = 43 + 4 * i;
			memcpy(tempGate -> inputIDs + i, serialGW + j, 4);
		}

		tempGate -> encOutputTable = (unsigned char**) calloc(tempGate -> outputTableSize, sizeof(unsigned char*));
		for(i = 0; i < tempGate -> outputTableSize; i ++)
		{
			j = 43 + (4 * tempGate -> numInputs) + (32 * i);
			tempGate -> encOutputTable[i] = (unsigned char*) calloc(32, sizeof(unsigned char));
			memcpy((tempGate -> encOutputTable) + i, serialGW + j, 32);
		}

		numInputs = tempGate -> numInputs;
		outputTableSize = tempGate -> outputTableSize;
	}

	if(0x0F == serialGW[5] || 0x00 == serialGW[6])
	{
		j = 43 + (4 * numInputs) + (32 * outputTableSize);
		toReturn -> outputWire -> wirePerm = serialGW[j];
	}
	toReturn -> gatePayload = tempGate;

	return toReturn;
}



struct bitsGarbleKeys *generateGarbleKeyPair(unsigned char perm)
{
	struct bitsGarbleKeys *toReturn = (struct bitsGarbleKeys*) calloc(1, sizeof(struct bitsGarbleKeys));

	toReturn -> key0 = generateRandBytes(16, 17);
	toReturn -> key0[16] = 0x00 ^ (0x01 & perm);
	
	toReturn -> key1 = generateRandBytes(16, 17);
	toReturn -> key1[16] = 0x01 ^ (0x01 & perm);

	return toReturn;
}


unsigned char getPermutation()
{
	unsigned char *toOutput = generateRandBytes(1, 1);

	return *toOutput;
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


void printGateOrWire(struct gateOrWire *input)
{
	if( NULL == input -> gatePayload )
	{
		printf("Wire ID  = %d\n", input -> G_ID);
	}
	else
	{
		printf("Gate ID  = %d\n", input -> G_ID);
		printGate( input -> gatePayload );
	}

	printf("Value = %d\n", input -> outputWire -> wirePermedValue);
}


struct gate *processGate(char* line, int strIndex, struct gateOrWire **circuit,
						struct gateOrWire *curGate)
{
	struct gate *toReturn = (struct gate*) calloc(1, sizeof(struct gate));
	int tempIndex, outputTableSize = 1;

	while( line[++ strIndex] != ' ' ) {}
	while( line[++ strIndex] != ' ' ) {}
	tempIndex = ++ strIndex;
	while( line[strIndex] != ' ' ) { strIndex ++; }

	char *tempString = (char*) calloc((strIndex - tempIndex + 1), sizeof(char));
	strncpy(tempString, line + tempIndex, (strIndex - tempIndex));
	toReturn -> numInputs = atoi(tempString);

	for(tempIndex = 0; tempIndex < toReturn -> numInputs; tempIndex ++)
		outputTableSize *= 2;

	toReturn -> outputTableSize = outputTableSize;
	toReturn -> rawOutputTable = parseOutputTable(line, &strIndex, toReturn);
	toReturn -> inputIDs = parseInputTable(line, toReturn -> numInputs, &strIndex);
	toReturn -> encOutputTable = recursiveOutputTable(toReturn);
	
	return toReturn;
}


struct gateOrWire *processGateOrWire(char *line, int idNum, int *strIndex, struct gateOrWire **circuit)
{
	struct gateOrWire *toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));
	int i, *rawOutputTable;

	toReturn -> G_ID = idNum;
	toReturn -> outputWire = (struct wire *) calloc(1, sizeof(struct wire));
	toReturn -> outputWire -> wirePerm = getPermutation();
	toReturn -> outputWire -> outputGarbleKeys = generateGarbleKeyPair(toReturn -> outputWire -> wirePerm);

	if( 'i' == line[*strIndex] )
	{
		toReturn -> gatePayload = NULL;
		toReturn -> outputWire -> wireMask = 0xF0;
	}
	else
	{
		if('o' == line[*strIndex])
		{
			toReturn -> outputWire -> wireMask = 0x0F;
			while( line[*strIndex] != ' ' )
			{
				*strIndex = *strIndex + 1;
			}
		}

		toReturn -> gatePayload = processGate(line, *strIndex, circuit, toReturn);
		encWholeOutTable(toReturn, circuit);
	}

	return toReturn;
}


void testSerialisation(struct gateOrWire *inputGW)
{
	unsigned char *serialGW;
	int serialLength = 0, i, j;
	struct gateOrWire *outputGW;
	unsigned char *temp;


	serialGW = serialiseGateOrWire(inputGW, &serialLength);
	outputGW = deserialiseGateOrWire(serialGW);

	printf("\nBefore serialisation\n");
	printf("G_ID				=  %d\n", inputGW -> G_ID);
	printf("Wire Mask			=  %02X\n", inputGW -> outputWire -> wireMask);
	printf("Wire Owner			=  %02X\n", inputGW -> outputWire -> wireOwner);
	printf("Wire Perm			=  %02X\n", inputGW -> outputWire -> wirePerm);

	if(0xFF == inputGW -> outputWire -> wireOwner)
	{
		printf("Out key 			=  ");
		for(i = 0; i < 32; i ++)
			printf("%02X", inputGW -> outputWire -> wireOutputKey[i]);
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
			memcpy(temp, (inputGW -> gatePayload -> encOutputTable) + i, 32);
			for(j = 0; j < 32; j ++)
			{
				printf("%02X", temp[j]);
			}
			printf("\n");
		}
	}


	printf("\nSerialised and back again\n");
	printf("G_ID				=  %d\n", outputGW -> G_ID);
	printf("Wire Mask			=  %02X\n", outputGW -> outputWire -> wireMask);
	printf("Wire Owner			=  %02X\n", outputGW -> outputWire -> wireOwner);
	printf("Wire Perm			=  %02X\n", outputGW -> outputWire -> wirePerm);

	if(0xFF == outputGW -> outputWire -> wireOwner)
	{
		printf("Out key 			=  ");
		for(i = 0; i < 32; i ++)
			printf("%02X", outputGW -> outputWire -> wireOutputKey[i]);
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
			memcpy(temp, (outputGW -> gatePayload -> encOutputTable) + i, 32);
			for(j = 0; j < 32; j ++)
			{
				printf("%02X", temp[j]);
			}
			printf("\n");
		}
	}
}