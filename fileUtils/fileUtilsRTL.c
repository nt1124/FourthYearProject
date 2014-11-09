
/*
int getNumGW(char *line)
{
	int strIndex = 0;
	int toReturn = 0;

	while( line[++ strIndex] != ' ' ) {}
	while( line[++ strIndex] == ' ' ) {}

	while( line[strIndex] != ' ')
	{
		toReturn *= 10;
		toReturn += atoi(line[strIndex++]);
	}

	return toReturn;
}
*/


// Returns an integer value when given a string with index pointing to start
// of the integer to read. Assumes delimtation by spaces.
int getIntFromString(char *inputStr, int& strIndex)
{
	int toReturn = 0;

	while(' ' != inputStr[strIndex])
	{
		toReturn *= 10;
		toReturn += (inputStr[strIndex] - 48);
		strIndex ++;
	}

	while(' ' == inputStr[strIndex])
	{
		strIndex ++;
	}

	return toReturn;
}


// RTL means the function deals with the Smart/Tillich style input.


struct gate *processGateRTL(int numInputWires, int *inputIDs, char gateType)
{
	struct gate *toReturn = (struct gate*) calloc(1, sizeof(struct gate));
	int i, outputTableSize = 1;

	toReturn -> numInputs = numInputWires;
	toReturn -> inputIDs = inputIDs;
	for(i = 0; i < toReturn -> numInputs; i ++)
		outputTableSize *= 2;

	toReturn -> outputTableSize = outputTableSize;
	if('I' == gateType)
	{
		toReturn -> rawOutputTable = (int*) calloc(2, sizeof(int));
		toReturn -> rawOutputTable[1] = 1;
	}
	else
	{
		toReturn -> rawOutputTable = (int*) calloc(4, sizeof(int));
		if('A' == gateType)
		{
			toReturn -> rawOutputTable[3] = 1;
		}
		else if('X' == gateType)
		{
			toReturn -> rawOutputTable[1] = 1;
			toReturn -> rawOutputTable[2] = 1;
		}
	}

	toReturn -> encOutputTable = recursiveOutputTable(toReturn);


	return toReturn;
}


struct gateOrWire *processGateOrWireRTL(int idNum, int *inputIDs, int numInputWires,
										char gateType, struct gateOrWire **circuit)
{
	struct gateOrWire *toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));

	toReturn -> G_ID = idNum;
	toReturn -> outputWire = (struct wire *) calloc(1, sizeof(struct wire));
	toReturn -> outputWire -> wirePerm = getPermutation();
	toReturn -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));
	toReturn -> outputWire -> outputGarbleKeys = generateGarbleKeyPair(toReturn -> outputWire -> wirePerm);

	toReturn -> gatePayload = processGateRTL(numInputWires, inputIDs, gateType);
	encWholeOutTable(toReturn, circuit);

	return toReturn;
}


struct gateOrWire *processGateLineRTL(char *line, struct gateOrWire **circuit)
{
	int strIndex = 0, idNum, i;
	int numInputWires, purposelessNumber;
	int *inputIDs;

	numInputWires = getIntFromString(line, strIndex);

	// As of yet unsure of the purpose of this number. Ask Nigel.
	purposelessNumber = getIntFromString(line, strIndex);

	inputIDs = (int*) calloc(numInputWires, sizeof(int));
	for(i = 0; i < numInputWires; i ++)
	{
		inputIDs[i] = getIntFromString(line, strIndex);
	}

	idNum = getIntFromString(line, strIndex);

	return processGateOrWireRTL(idNum, inputIDs, numInputWires, line[strIndex], circuit);
}


struct gateOrWire *initialiseInputWire(int idNum, unsigned char owner)
{
	struct gateOrWire *toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));

	toReturn -> G_ID = idNum;
	toReturn -> outputWire = (struct wire *) calloc(1, sizeof(struct wire));
	toReturn -> outputWire -> wirePerm = getPermutation();
	toReturn -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));
	toReturn -> outputWire -> outputGarbleKeys = generateGarbleKeyPair(toReturn -> outputWire -> wirePerm);
	toReturn -> gatePayload = NULL;
	toReturn -> outputWire -> wireMask = 0xF0;
	toReturn -> outputWire -> wireOwner = owner;

	return toReturn;
}


// FOR NOW we assume party 1 is building the circuit.
struct gateOrWire **initialiseAllInputs(int numGates, int numInputs1, int numInputs2)
{
	struct gateOrWire **circuit = (struct gateOrWire**) calloc(numGates, sizeof(struct gateOrWire*));
	int i;

	for(i = 0; i < numInputs1; i ++)
	{
		circuit[i] = initialiseInputWire(i, 0xFF);
	}

	for(i = numInputs2; i < numInputs1 + numInputs2; i ++)
	{
		circuit[i] = initialiseInputWire(i, 0x00);
	}

	return circuit;
}



struct gateOrWire **readInCircuiRTL(char* filepath, int *numGates)
{
	FILE *file = fopen ( filepath, "r" );
	char line [ 512 ]; // Or other suitable maximum line size
	int numInputs1, numInputs2, numOutputs;	int gateIndex = 0;
	struct gateOrWire *tempGateOrWire;
	struct gateOrWire **circuit;
	int i;

	if ( file != NULL )
	{
		if(NULL != fgets(line, sizeof(line), file))
			sscanf(line, "%d %d", &numInputs1, numGates);
		else
			return NULL;

		if(NULL != fgets(line, sizeof(line), file))
			sscanf(line, "%d %d\t%d", &numInputs1, &numInputs2, &numOutputs);
		else
			return NULL;

		if(NULL == fgets(line, sizeof(line), file))
			return NULL;

		circuit = initialiseAllInputs(*numGates, numInputs1, numInputs2);

		while ( fgets(line, sizeof(line), file) != NULL ) // Read a line
		{
			tempGateOrWire = processGateLineRTL(line, circuit);
			if( NULL != tempGateOrWire )
			{
				gateIndex = tempGateOrWire -> G_ID;
				*(circuit + gateIndex) = tempGateOrWire;
			}
		}

		for(i = 0; i < numOutputs; i ++)
		{
			gateIndex = numGates - i - 1;
			circuit[gateIndex] -> outputWire -> wireMask = 0x0F;
		}
		fclose ( file );
	}

	return circuit;
}