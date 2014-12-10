void printGateOrWire(struct gateOrWire *inputGW)
{
	int i, j;
	unsigned char *temp;

	printf("G_ID                    =  %d\n", inputGW -> G_ID);
	printf("Wire Mask               =  %02X\n", inputGW -> outputWire -> wireMask);
	printf("Wire Owner              =  %02X\n", inputGW -> outputWire -> wireOwner);
	printf("Wire Perm               =  %02X\n", inputGW -> outputWire -> wirePerm);
	printf("Wire Value              =  %02X\n", inputGW -> outputWire -> wirePermedValue);
	printf("Out key                 =  ");
	for(i = 0; i < 16; i ++)
		printf("%02X", inputGW -> outputWire -> wireOutputKey[i]);
	printf("\n");
	fflush(stdout);


	if(0x00 == inputGW -> outputWire -> wireOwner &&
	   0xF0 == inputGW -> outputWire -> wireMask)
	{
		printf("key0                    =  ");
		for(i = 0; i < 16; i ++)
			printf("%02X", inputGW -> outputWire -> outputGarbleKeys -> key0[i]);
		printf("\n");
		fflush(stdout);

		printf("key1                    =  ");
		for(i = 0; i < 16; i ++)
			printf("%02X", inputGW -> outputWire -> outputGarbleKeys -> key1[i]);
		printf("\n");
		fflush(stdout);
	}

	if(NULL != inputGW -> gatePayload)
	{
		printf("numInputs               =  %d\n", inputGW -> gatePayload -> numInputs);
		printf("outputTableSize         =  %d\n", inputGW -> gatePayload -> outputTableSize);
		fflush(stdout);

		for(i = 0; i < inputGW -> gatePayload -> numInputs; i ++)
			printf("inputIDs[%d]             =  %d\n", i, inputGW -> gatePayload -> inputIDs[i]);
		fflush(stdout);

		temp = (unsigned char*) calloc(32, sizeof(unsigned char));
		for(i = 0; i < inputGW -> gatePayload -> outputTableSize; i ++)
		{
			printf("encOutputTable[%d]       =  ", i);
			memcpy(temp, inputGW -> gatePayload -> encOutputTable[i], 32);
			for(j = 0; j < 32; j ++)
			{
				printf("%02X", temp[j]);
			}
			printf("\n");
		}
		fflush(stdout);
	}
}

unsigned char **recursiveOutputTable(struct gate *curGate)
{
	unsigned char **toReturn = (unsigned char**) calloc(curGate -> outputTableSize, sizeof(unsigned char*));
	int i;

	for(i = 0; i < curGate -> outputTableSize; i ++)
	{
		toReturn[i] = (unsigned char*) calloc(32, sizeof(unsigned char));
		toReturn[i][16] = curGate -> rawOutputTable[i];
	}

	return toReturn;
}


// Encrypt the whole of the output table for a given gate.
void encWholeOutTable(struct gateOrWire *curGate, struct gateOrWire **circuit)
{
	int numInputs = curGate -> gatePayload -> numInputs;
	int i, j, tempBit, permedIndex, inputID;
	struct wire *inputWire;
	unsigned char *keyList[numInputs], *tempRow;
	unsigned char *toEncrypt0 = (unsigned char*) calloc(32, sizeof(unsigned char));
	unsigned char *toEncrypt1 = (unsigned char*) calloc(32, sizeof(unsigned char));
	
	memcpy(toEncrypt0, curGate -> outputWire -> outputGarbleKeys -> key0, 17);
	memcpy(toEncrypt1, curGate -> outputWire -> outputGarbleKeys -> key1, 17);

	for(j = 0; j < numInputs; j ++)
	{
		keyList[j] = (unsigned char*) calloc(16, sizeof(unsigned char));
	}

	for(i = 0; i < curGate -> gatePayload -> outputTableSize; i ++)
	{
		permedIndex = 0;
		for(j = 0; j < numInputs; j ++)
		{
			inputID = curGate -> gatePayload -> inputIDs[j];
			inputWire = circuit[inputID] -> outputWire;

			tempBit = (i >> j) & 0x01;
			permedIndex ^= (inputWire -> wirePerm & 0x01) << j;

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
		free(tempRow);
	}

	for(j = 0; j < numInputs; j ++)
	{
		free(keyList[j]);
	}
	free(toEncrypt0);
	free(toEncrypt1);
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
	memcpy(curGate -> outputWire -> wireOutputKey, toReturn, 16);

	curGate -> outputWire -> wirePermedValue = toReturn[16];
	
	for(j = 0; j < numInputs; j ++)
	{
		free(keyList[j]);
	}
	free(toReturn);
}


int provideKeyForGate(struct gateOrWire *inputGW, int sockfd, mpz_t N)
{
	struct wire *tempWire = inputGW -> outputWire;

	// senderOT_Toy(sockfd, tempWire -> outputGarbleKeys -> key0, tempWire -> outputGarbleKeys -> key1, 16);
	senderOT_SH_RSA(sockfd, tempWire -> outputGarbleKeys -> key0, tempWire -> outputGarbleKeys -> key1, 16);

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
	unsigned char *toOutputPointer = generateRandBytes(1, 1);
	unsigned char toReturn = *toOutputPointer;
	free(toOutputPointer);

	return toReturn;
}


void freeGate(struct gate *inputGate)
{
	int i;

	if(NULL != inputGate)
	{
		free(inputGate -> inputIDs);

		for(i = 0; i < inputGate -> outputTableSize; i ++)
		{
			free(inputGate -> encOutputTable[i]);
			inputGate -> encOutputTable[i] = NULL;
		}

		if(NULL != inputGate)
		{
			free(inputGate -> encOutputTable);
			inputGate -> encOutputTable = NULL;
		}

		if(NULL != inputGate -> rawOutputTable)
			free(inputGate -> rawOutputTable);
		
		free(inputGate);
	}
}


void freeGateOrWire(struct gateOrWire *inputGW)
{
	freeGate(inputGW -> gatePayload);
	
	if(NULL != inputGW -> outputWire -> outputGarbleKeys)
	{
		free(inputGW -> outputWire -> outputGarbleKeys -> key0);
		free(inputGW -> outputWire -> outputGarbleKeys -> key1);
		free(inputGW -> outputWire -> outputGarbleKeys);
	}

	if(NULL != inputGW -> outputWire)
	{
		if(NULL != inputGW -> outputWire -> wireOutputKey)
			free(inputGW -> outputWire -> wireOutputKey);

		free(inputGW -> outputWire);
	}

	free(inputGW);
}
