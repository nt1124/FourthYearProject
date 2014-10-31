void printGateOrWire(struct gateOrWire *inputGW)
{
	int i, j;
	unsigned char *temp;

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

	}
}



void encWholeOutTable(struct gateOrWire *curGate, struct gateOrWire **circuit)
{
	int numInputs = curGate -> gatePayload -> numInputs;
	int i, j, tempBit, permedIndex, inputID;
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


void decryptGate(struct gateOrWire *curGate, struct gateOrWire **inputCircuit)
{
	int numInputs = curGate -> gatePayload -> numInputs;
	int j, tempBit, tempIndex, outputIndex = 0;
	unsigned char *keyList[numInputs], *toReturn;
	unsigned char *tempRow;

	for(j = 0; j < numInputs; j ++)
	{
		tempIndex = curGate -> gatePayload -> inputIDs[numInputs - j - 1];
		tempBit = inputCircuit[tempIndex] -> outputWire -> wirePermedValue;

		outputIndex = (outputIndex << 1) + tempBit;

		keyList[j] = (unsigned char*) calloc(16, sizeof(unsigned char));
		memcpy(keyList[j], inputCircuit[tempIndex] -> outputWire -> wireOutputKey, 16);
	}

	tempRow = curGate -> gatePayload -> encOutputTable[outputIndex];
	toReturn = decryptMultipleKeys(keyList, numInputs, tempRow, 2);
	curGate -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));
	memcpy(curGate -> outputWire -> wireOutputKey, toReturn, 16);

	curGate -> outputWire -> wirePermedValue = toReturn[16];

/*
	printf("--->  %02X\n", curGate -> outputWire -> wirePermedValue);
	for(j = 0; j < 16; j ++)
	{
		printf("%02X", curGate -> outputWire -> wireOutputKey[j]);
	} printf("\n");
*/
}


int provideKeyForGate(struct gateOrWire *inputGW, int sockfd)
{
	struct wire *tempWire = inputGW -> outputWire;

	senderOT_Toy(sockfd, tempWire -> outputGarbleKeys -> key0, tempWire -> outputGarbleKeys -> key1, 16);

	return 1;
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
	// int *rawOutputTable;

	toReturn -> G_ID = idNum;
	toReturn -> outputWire = (struct wire *) calloc(1, sizeof(struct wire));
	toReturn -> outputWire -> wirePerm = getPermutation();
	toReturn -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));
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

