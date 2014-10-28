

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

	// printf("++ %d\n", curGate -> G_ID);
	for(i = 0; i < curGate -> gatePayload -> outputTableSize; i ++)
	{	
		// printf("(");
		permedIndex = 0;
		for(j = 0; j < numInputs; j ++)
		{
			inputID = curGate -> gatePayload -> inputIDs[j];
			inputWire = circuit[inputID] -> outputWire;

			tempBit = i;
			tempBit = ((tempBit >> j) & 0x01);
			permedIndex <<= 1;
			permedIndex += (inputWire -> wirePerm & 0x01);
			// printf("%d ", (inputWire -> wirePerm & 0x01));

			keyList[j] = (unsigned char*) calloc(16, sizeof(unsigned char));
			if(0 == tempBit)
				memcpy(keyList[j], inputWire -> outputGarbleKeys -> key0, 16);
			else if(1 == tempBit)
				memcpy(keyList[j], inputWire -> outputGarbleKeys -> key1, 16);
		}

		permedIndex = permedIndex ^ i;
		// printf(") %d -> %d\n", i, permedIndex);

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
	int i, j, tempBit, tempIndex, outputIndex = 0;
	unsigned char *keyList[numInputs], *toReturn;
	unsigned char *tempRow;

	// printf("==  %d\n", curGate -> G_ID);
	for(j = 0; j < numInputs; j ++)
	{
		tempIndex = curGate -> gatePayload -> inputIDs[j];
		tempBit = inputCircuit[tempIndex] -> outputWire -> wirePermedValue;

		// tempBit = tempBit ^ (0x01 & inputCircuit[tempIndex] -> outputWire -> wirePerm);
		outputIndex <<= 1;
		outputIndex += tempBit;

		keyList[numInputs - j - 1] = (unsigned char*) calloc(16, sizeof(unsigned char));
		memcpy(keyList[numInputs - j - 1], inputCircuit[tempIndex] -> outputWire -> wireOutputKey, 16);
	}
	// If we have inputIDs [a b c] c is LSB, a is MSB

	tempRow = curGate -> gatePayload -> encOutputTable[outputIndex];
	toReturn = decryptMultipleKeys(keyList, numInputs, tempRow, 2);
	curGate -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));
	memcpy(curGate -> outputWire -> wireOutputKey, toReturn, 16);

/*
	printf("%d - %d - ", curGate -> G_ID, outputIndex);
	for(i = 0; i < 17; i ++)
		printf("%02X.", toReturn[i]);
	printf("%02X", (toReturn[i] ^ ((0x01 & curGate -> outputWire -> wirePerm)) ) );
	printf("\n");
	fflush(stdout);
*/
	// curGate -> outputWire -> wirePermedValue = toReturn[16];
	// curGate -> gatePayload -> rawOutputTable[outputIndex];
}


unsigned char *serialiseGateOrWire(struct gateOrWire *inputGW, int *outputLength)
{
	unsigned char *toReturn;
	int i, j, outputTableSize = inputGW -> gatePayload -> outputTableSize;
	int numInputs = inputGW -> gatePayload -> numInputs;
	int serialisedSize = 42 + (32 * outputTableSize) + (4 * numInputs);

	toReturn = (unsigned char*) calloc(serialisedSize, sizeof(unsigned char));

	memcpy(toReturn, &(inputGW -> G_ID), 4);
	if(NULL == inputGW -> gatePayload)
		toReturn[4] = 0x00;
	else
		toReturn[4] = 0xFF;

	toReturn[5] = inputGW -> outputWire -> wireMask;
	toReturn[6] = inputGW -> outputWire -> wirePermedValue;
	memcpy(toReturn + 7, inputGW -> outputWire -> wireOutputKey, 32);
	*outputLength = 39;

	if(NULL != inputGW -> gatePayload)
	{
		toReturn[39] = inputGW -> gatePayload -> numInputs;
		memcpy(toReturn + 40, &outputTableSize, 2);
		for(i = 0; i < inputGW -> gatePayload -> numInputs; i ++)
		{
			j = 42 + 4 * i;
			memcpy(toReturn + j, inputGW -> gatePayload -> inputIDs + i, 4);
		}

		for(i = 0; i < outputTableSize; i ++)
		{
			j = 42 + 4 * numInputs + 32 * i;
			memcpy(toReturn + j, inputGW -> gatePayload -> encOutputTable + i, 32);
		}
		//memcpy(toReturn + 42 , inputGW -> gatePayload -> encOutputTable, 32 * outputTableSize);
		*outputLength += ( (32 * outputTableSize) + (4 * numInputs) + 3 );
	}

	return toReturn;
}


struct gateOrWire *deserialiseGateOrWire(unsigned char *serialGW, int outputLength)
{
	struct gateOrWire *toReturn;
	struct gate *tempGate;
	int i, j;

	toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));
	toReturn -> outputWire = (struct wire*) calloc(1, sizeof(struct wire));

	memcpy(&(toReturn -> G_ID), serialGW, 4);
	toReturn -> outputWire -> wireMask = serialGW[5];
	toReturn -> outputWire -> wirePermedValue = serialGW[6];
	memcpy(toReturn -> outputWire -> wireOutputKey, serialGW + 7, 32);

	if(0xFF == serialGW[4])
	{
		tempGate = (struct gate*) calloc(1, sizeof(struct gate));
		tempGate -> numInputs = serialGW[39];
		memcpy(&(tempGate -> outputTableSize), serialGW + 40, 2);

		tempGate -> inputIDs = (int*) calloc(tempGate -> numInputs, sizeof(int));
		for(i = 0; i < tempGate -> numInputs; i ++)
		{
			j = 42 + 4 * i;
			memcpy(tempGate -> inputIDs + i, serialGW + j, 4);
		}

		for(i = 0; i < tempGate -> outputTableSize; i ++)
		{
			j = 42 + (4 * tempGate -> numInputs) + (32 * i);
			tempGate -> encOutputTable[i] = (unsigned char*) calloc(32, sizeof(unsigned char));
			memcpy(tempGate -> encOutputTable + i, toReturn + j, 32);
		}
	}

	return toReturn;
}



struct bitsGarbleKeys *generateGarbleKeyPair(unsigned char perm)
{
	struct bitsGarbleKeys *toReturn = (struct bitsGarbleKeys *)calloc(1, sizeof(struct bitsGarbleKeys));

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
	{
		outputTableSize *= 2;
	}

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
