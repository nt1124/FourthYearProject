void encWholeOutTable(struct gateOrWire *curGate, struct gateOrWire **circuit, int *rawOutputTable)
{
	int numInputs = curGate -> gatePayload -> numInputs;
	unsigned char *keyList[numInputs];
	unsigned char *tempRow;
	int i, j, k, tempBit, permutatedIndex, inputID;
	struct wire *inputWire;

	unsigned char *toEncrypt0 = (unsigned char*) calloc(32, sizeof(unsigned char));
	unsigned char *toEncrypt1 = (unsigned char*) calloc(32, sizeof(unsigned char));
	memcpy( toEncrypt0, curGate -> outputWire -> outputGarbleKeys -> key0, 17);
	memcpy( toEncrypt1, curGate -> outputWire -> outputGarbleKeys -> key1, 17);

	for(i = 0; i < curGate -> gatePayload -> outputTableSize; i ++)
	{
		permutatedIndex = 0;
		for(j = 0; j < numInputs; j ++)
		{
			inputID = curGate -> gatePayload -> inputIDs[j];
			inputWire = circuit[inputID] -> outputWire;

			tempBit = ((i >> j) & 1);
			permutatedIndex <<= 2;
			permutatedIndex += tempBit ^ (inputWire -> wirePerm & 0x01);

			// Got a seg fault somewhere in these lines.
			if(0 == tempBit)
				memcpy(keyList[j], inputWire -> outputGarbleKeys -> key0, 16);
			else if(1 == tempBit)
				memcpy(keyList[j], inputWire -> outputGarbleKeys -> key1, 16);
			printf("....>");
			fflush(stdout);
		}

		printf("<>\n");
		fflush(stdout);

		tempRow = curGate -> gatePayload -> encOutputTable[i];
		if(0 == *(rawOutputTable + i))
			tempRow = encryptMultipleKeys(keyList, numInputs, toEncrypt0, 2);
		else if(1 == *(rawOutputTable + i))
			tempRow = encryptMultipleKeys(keyList, numInputs, toEncrypt1, 2);
	}

}


unsigned char *decryptGate(struct gateOrWire *curGate, struct gateOrWire **inputCircuit)
{
	int numInputs = curGate -> gatePayload -> numInputs;
	int i, j, tempBit, tempIndex, outputIndex = 0;
	unsigned char *keyList[numInputs], *toReturn;
	unsigned char *tempRow;

	for(j = 0; j < numInputs; j ++)
	{
		tempIndex = curGate -> gatePayload -> inputIDs[j];

		tempBit = inputCircuit[tempIndex] -> outputWire -> wirePermedValue;
		tempBit = ((tempBit >> j) & 1);
		outputIndex <<= 1;
		outputIndex += tempBit;

		memcpy(keyList[j], inputCircuit[tempIndex] -> outputWire -> wireOutputKey, 16);
		/*
		if(0 == tempBit)
			keyList[j] = curGate -> gate_data -> inputKeySet[j] -> key0;
		else if(1 == tempBit)
			keyList[j] = curGate -> gate_data -> inputKeySet[j] -> key1;
		*/
	}

	tempRow = curGate -> gatePayload -> encOutputTable[outputIndex];
	toReturn = decryptMultipleKeys(keyList, numInputs, tempRow, 2);

	return toReturn;
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
						struct gateOrWire *curGate, int *rawOutputTable)
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
	rawOutputTable = parseOutputTable(line, &strIndex, toReturn);
	toReturn -> inputIDs = parseInputTable(line, toReturn -> numInputs, &strIndex);
	toReturn -> encOutputTable = recursiveOutputTable(rawOutputTable, toReturn);
	
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

		toReturn -> gatePayload = processGate(line, *strIndex, circuit, toReturn, rawOutputTable);
		encWholeOutTable(toReturn, circuit, rawOutputTable);
	}

	return toReturn;
}
