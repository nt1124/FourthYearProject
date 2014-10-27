/*
void recursiveEncryptionTree(struct gateOrWire *curGate)
{
	unsigned char *keyList[curGate -> gatePayload -> numInputs];
	unsigned char *tempRow;
	int i, j, k, tempBit;

	for(i = 0; i < curGate -> gatePayload -> outputTableSize; i ++)
	{
		tempRow = curGate -> gatePayload -> encOutputTable[i];
		for(j = 0; j < curGate -> gatePayload -> numInputs; j ++)
		{
			tempBit = ((i >> j) & 1);
			//  ^ ((curGate -> outputWire -> tablePermutation >> j) & 1);

			if(0 == tempBit)
				keyList[j] = curGate -> gatePayload -> inputKeySet[j] -> key0;
			else if(1 == tempBit)
				keyList[j] = curGate -> gatePayload -> inputKeySet[j] -> key1;
		}

		if(0 == tempRow -> outputValue)
			tempRow -> outputEncValue = encryptMultipleKeys(keyList, curGate -> gatePayload -> numInputs, curGate -> outputGarbleKeys -> key0, 1);
		else if(1 == tempRow -> outputValue)
			tempRow -> outputEncValue = encryptMultipleKeys(keyList, curGate -> gatePayload -> numInputs, curGate -> outputGarbleKeys -> key1, 1);
	}
}


void encryptOutputTable(struct gateOrWire *curGate)
{
	unsigned char *keyList[curGate -> gatePayload -> numInputs];
	struct outputEncRow *tempRow;
	int i, j, k, tempBit, permutatedIndex;

	for(i = 0; i < curGate -> gatePayload -> outputTableSize; i ++)
	{
		permutatedIndex = 0;
		for(j = 0; j < curGate -> gatePayload -> numInputs; j ++)
		{
			tempBit = ((i >> j) & 1);
			permutatedIndex <<= 2;
			permutatedIndex += tempBit ^ ((curGate -> gate_data -> tablePermutation >> j) & 1);

			if(0 == tempBit)
				keyList[j] = curGate -> gate_data -> inputKeySet[j] -> key0;
			else if(1 == tempBit)
				keyList[j] = curGate -> gate_data -> inputKeySet[j] -> key1;
		}

		tempRow = curGate -> gatePayload -> outputTreeEnc[permutatedIndex];
		if(0 == tempRow -> outputValue)
			tempRow -> outputEncValue = encryptMultipleKeys(keyList, curGate -> gatePayload -> numInputs, curGate -> outputGarbleKeys -> key0, 1);
		else if(1 == tempRow -> outputValue)
			tempRow -> outputEncValue = encryptMultipleKeys(keyList, curGate -> gatePayload -> numInputs, curGate -> outputGarbleKeys -> key1, 1);
	}
}


unsigned char *decryptionTree(struct gateOrWire *curGate, struct gateOrWire **inputCircuit)
{
	unsigned char *keyList[curGate -> gate_data -> numInputs];
	struct outputEncRow *tempRow;
	int i, j, k, tempBit, tempIndex, outputIndex = 0;

	for(j = 0; j < curGate -> gate_data -> numInputs; j ++)
	{
		tempIndex = curGate -> gate_data -> inputIDs[j];

		tempBit = inputCircuit[tempIndex] -> wireValue;
		tempBit = ((tempBit >> j) & 1) ^ ((curGate -> gate_data -> tablePermutation >> j) & 1);
		outputIndex <<= 1;
		outputIndex += tempBit;

		if(0 == tempBit)
			keyList[j] = curGate -> gate_data -> inputKeySet[j] -> key0;
		else if(1 == tempBit)
			keyList[j] = curGate -> gate_data -> inputKeySet[j] -> key1;
	}

	tempRow = curGate -> gate_data -> outputTreeEnc[outputIndex];
	curGate -> wireValue = tempRow -> outputValue;
	if(0 == tempRow -> outputValue)
		return decryptMultipleKeys(keyList, curGate -> gate_data -> numInputs, tempRow -> outputEncValue, 1);
	else if(1 == tempRow -> outputValue)
		return decryptMultipleKeys(keyList, curGate -> gate_data -> numInputs, tempRow -> outputEncValue, 1);
}
*/


struct bitsGarbleKeys *generateGarbleKeyPair()
{
	struct bitsGarbleKeys *toReturn = (struct bitsGarbleKeys *)calloc(1, sizeof(struct bitsGarbleKeys));

	toReturn -> key0 = generateRandBytes(16);
	toReturn -> key1 = generateRandBytes(16);

	return toReturn;
}


int *getInputGarbleKeys(struct gateOrWire **circuit, int numInputs, int *inputIDs)
{
	// struct bitsGarbleKeys **inputKeys = (struct bitsGarbleKeys **)calloc(numInputs, sizeof(struct bitsGarbleKeys *));
	int i, gid, *inputKeys = (int*) calloc(numInputs, sizeof(int));

	for(i = 0; i < numInputs; i ++)
	{
		gid = *(inputIDs + i);
		inputKeys[i] = gid;
		//circuit[gid] -> outputGarbleKeys;
	}

	return inputKeys;
}


unsigned char getPermutation()
{
	unsigned char *toOutput = generateRandBytes(1);

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

	printf("Value = %d\n", input -> outputWire -> wireValue);
}


struct gate *processGate(char* line, int strIndex, struct gateOrWire **circuit, struct gateOrWire *curGate)
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
	// toReturn -> inputKeySet = getInputGarbleKeys(circuit, toReturn -> numInputs, toReturn -> inputIDs);
	toReturn -> encOutputTable = recursiveOutputTable(tableToParse, toReturn);
	// toReturn -> tablePermutation = getPermutation();
	
	return toReturn;
}


struct gateOrWire *processGateOrWire(char *line, int idNum, int *strIndex, struct gateOrWire **circuit)
{
	struct gateOrWire *toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));
	int i;

	toReturn -> G_ID = idNum;
	toReturn -> outputWire = (struct wire *) calloc(1, sizeof(struct wire));
	toReturn -> outputWire -> outputGarbleKeys = generateGarbleKeyPair();
	toReturn -> outputWire -> wirePerm = getPermutation();

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
		// recursiveEncryptionTree(toReturn, circuit);
	}

	return toReturn;
}
