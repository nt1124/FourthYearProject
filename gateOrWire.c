unsigned char *serialiseGateOrWire(struct gateOrWire *toSerialise)
{
	unsigned char *toReturn;

	

	return toReturn;
}


void recursiveEncryptionTree(struct gateOrWire *curGate)
{
	unsigned char *keyList[curGate -> gate_data -> numInputs];
	struct outputEncRow *tempRow;
	int i, j, k, tempBit;

	for(i = 0; i < curGate -> gate_data -> outputTableSize; i ++)
	{
		tempRow = curGate -> gate_data -> outputTreeEnc;
		for(j = 0; j < curGate -> gate_data -> numInputs; j ++)
		{
			tempBit = (i >> j) & 1;
			tempRow = tempRow -> keyChoice[tempBit];

			if(0 == tempBit)
				keyList[j] = curGate -> gate_data -> inputKeySet[j] -> key0;
			else if(1 == tempBit)
				keyList[j] = curGate -> gate_data -> inputKeySet[j] -> key1;
		}

		if(0 == tempRow -> outputValue)		
			tempRow -> outputEncValue = encryptMultiple(keyList, curGate -> gate_data -> numInputs, curGate -> outputGarbleKeys -> key0);
		else if(1 == tempRow -> outputValue)
			tempRow -> outputEncValue = encryptMultiple(keyList, curGate -> gate_data -> numInputs, curGate -> outputGarbleKeys -> key1);
	}
}


unsigned char *decryptionTree(struct gateOrWire *curGate, struct gateOrWire **inputCircuit)
{
	unsigned char *keyList[curGate -> gate_data -> numInputs];
	struct outputEncRow *tempRow;
	int i, j, k, tempBit, tempIndex;

	tempRow = curGate -> gate_data -> outputTreeEnc;
	for(j = 0; j < curGate -> gate_data -> numInputs; j ++)
	{
		tempIndex = curGate -> gate_data -> inputIDs[j];
		tempBit = inputCircuit[tempIndex] -> wireValue;
		tempRow = tempRow -> keyChoice[tempBit];

		if(0 == tempBit)
			keyList[j] = curGate -> gate_data -> inputKeySet[j] -> key0;
		else if(1 == tempBit)
			keyList[j] = curGate -> gate_data -> inputKeySet[j] -> key1;
	}

	curGate -> wireValue = tempRow -> outputValue;
	if(0 == tempRow -> outputValue)
		return decryptMultiple(keyList, curGate -> gate_data -> numInputs, tempRow -> outputEncValue);
	else if(1 == tempRow -> outputValue)
		return decryptMultiple(keyList, curGate -> gate_data -> numInputs, tempRow -> outputEncValue);
}


struct bitsGarbleKeys *generateGarbleKeyPair()
{
	struct bitsGarbleKeys *toReturn = (struct bitsGarbleKeys *)calloc(1, sizeof(struct bitsGarbleKeys));

	// RAND_bytes(toReturn -> key0, 16);
	// RAND_bytes(toReturn -> key1, 16);
	toReturn -> key0 = generateRandBytes(16);
	toReturn -> key1 = generateRandBytes(16);

	return toReturn;
}


struct bitsGarbleKeys **getInputGarbleKeys(struct gateOrWire **circuit, int numInputs, int *inputIDs)
{
	struct bitsGarbleKeys **inputKeys = (struct bitsGarbleKeys **)calloc(numInputs, sizeof(struct bitsGarbleKeys *));
	int i, gid;

	for(i = 0; i < numInputs; i ++)
	{
		gid = *(inputIDs + i);
		inputKeys[i] = circuit[gid] -> outputGarbleKeys;
	}

	return inputKeys;
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
	if( NULL == input -> gate_data )
	{
		printf("Wire ID  = %d\n", input -> G_ID);
	}
	else
	{
		printf("Gate ID  = %d\n", input -> G_ID);
		printGate( input -> gate_data );
	}

	printf("Value = %d\n", input -> wireValue);
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
	toReturn -> inputKeySet = getInputGarbleKeys(circuit, toReturn -> numInputs, toReturn -> inputIDs);
	toReturn -> outputTreeEnc = recursiveOutputTable(tableToParse, toReturn);
	
	return toReturn;
}


struct gateOrWire *processGateOrWire(char *line, int idNum, int *strIndex, struct gateOrWire **circuit)
{
	struct gateOrWire *toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));
	int i;

	toReturn -> G_ID = idNum;
	toReturn -> outputGarbleKeys = generateGarbleKeyPair();

	if( 'i' == line[*strIndex] )
	{
		// toReturn -> typeTag = 'W';
		toReturn -> gate_data = NULL;
	}
	else
	{
		// toReturn -> typeTag = 'G';
		if('o' == line[*strIndex])
		{
			toReturn -> outputFlag = 1;
			while( line[*strIndex] != ' ' )
			{
				*strIndex = *strIndex + 1;
			}
		}

		toReturn -> gate_data = processGate(line, *strIndex, circuit, toReturn);
		recursiveEncryptionTree(toReturn);
	}

	return toReturn;
}
