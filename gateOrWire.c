void encWholeOutTable(struct gateOrWire *curGate, struct gateOrWire **circuit)
{
	int numInputs = curGate -> gatePayload -> numInputs;
	unsigned char *keyList[numInputs], *tempRow;
	int i, j, k, tempBit, permedIndex, inputID;
	struct wire *inputWire;

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

			tempBit = ((i >> j) & 1);
			permedIndex <<= 2;
			permedIndex += tempBit ^ (inputWire -> wirePerm & 0x01);

			keyList[j] = (unsigned char*) calloc(16, sizeof(unsigned char));
			if(0 == tempBit)
				memcpy(keyList[j], inputWire -> outputGarbleKeys -> key0, 16);
			else if(1 == tempBit)
				memcpy(keyList[j], inputWire -> outputGarbleKeys -> key1, 16);
		}

		// printf("%d == %d\n", curGate -> G_ID, i);
		if(0 == curGate -> gatePayload -> rawOutputTable[i])
			tempRow = encryptMultipleKeys(keyList, numInputs, toEncrypt0, 2);
		else if(1 == curGate -> gatePayload -> rawOutputTable[i])
			tempRow = encryptMultipleKeys(keyList, numInputs, toEncrypt1, 2);

		/*
		for(k = 0; k < 32; k ++)
		{
			printf("%02X ", tempRow[k]);
		}
		printf("\n");
		*/
		memcpy(curGate -> gatePayload -> encOutputTable[i], tempRow, 32);
	}	

}


void decryptGate(struct gateOrWire *curGate, struct gateOrWire **inputCircuit)
{
	int numInputs = curGate -> gatePayload -> numInputs;
	int i, j, tempBit, tempIndex, outputIndex = 0;
	unsigned char *keyList[numInputs], *toReturn;
	unsigned char *tempRow;

	for(j = 0; j < numInputs; j ++)
	{
		tempIndex = curGate -> gatePayload -> inputIDs[j];

		tempBit = inputCircuit[tempIndex] -> outputWire -> wirePermedValue;

		outputIndex <<= 1;
		outputIndex += tempBit;

		keyList[numInputs - j - 1] = (unsigned char*) calloc(16, sizeof(unsigned char));
		memcpy(keyList[numInputs - j - 1], inputCircuit[tempIndex] -> outputWire -> wireOutputKey, 16);
	}

	tempRow = curGate -> gatePayload -> encOutputTable[outputIndex];

	// printf("%d ++ %d\n", curGate -> G_ID, outputIndex);
	toReturn = decryptMultipleKeys(keyList, numInputs, tempRow, 2);
	
	curGate -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));
	memcpy(curGate -> outputWire -> wireOutputKey, toReturn, 16);
	// printf("%d -> %d\n", curGate -> G_ID, outputIndex);
	// Below line causes seg fault if proper input is uncommented.
	curGate -> outputWire -> wirePermedValue = curGate -> gatePayload -> rawOutputTable[outputIndex];//toReturn[16];
}


struct bitsGarbleKeys *generateGarbleKeyPair(unsigned char perm)
{
	struct bitsGarbleKeys *toReturn = (struct bitsGarbleKeys *)calloc(1, sizeof(struct bitsGarbleKeys));

	toReturn -> key0 = generateRandBytes(16, 17);
	toReturn -> key0[16] = 0x00;// ^ (0x01 & perm);
	toReturn -> key1 = generateRandBytes(16, 17);
	toReturn -> key1[16] = 0x01;// ^ (0x01 & perm);

	return toReturn;
}

/*
int *getInputGarbleKeys(struct gateOrWire **circuit, int numInputs, int *inputIDs)
{
	int i, gid, *inputKeys = (int*) calloc(numInputs, sizeof(int));

	for(i = 0; i < numInputs; i ++)
	{
		gid = *(inputIDs + i);
		inputKeys[i] = gid;
		//circuit[gid] -> outputGarbleKeys;
	}

	return inputKeys;
}
*/

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
