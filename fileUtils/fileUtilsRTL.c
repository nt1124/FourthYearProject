
// Function for use in testing, will set all inputs to zero and
// initialise accordingly.
void zeroAllInputs(struct gateOrWire **inputCircuit, int numGates)
{
	struct wire* outputWire;
	int i;

	for(i = 0; i < numGates; i ++)
	{
		outputWire = inputCircuit[i] -> outputWire;

		outputWire -> wireOwner = 0xFF;
		outputWire -> wirePermedValue = outputWire -> outputGarbleKeys -> key0[16];
		memcpy(outputWire -> wireOutputKey, outputWire -> outputGarbleKeys -> key0, 16);
		
		if( 0x01 != (0x0F & inputCircuit[i] -> outputWire -> wireMask) )
			break;
	}
}


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
		toReturn -> rawOutputTable[0] = 1;
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

	toReturn -> encOutputTable = createOutputTable(toReturn);


	return toReturn;
}


// Process a gateOrWire struct given the data.
struct gateOrWire *processGateOrWireRTL(int idNum, int *inputIDs, int numInputWires,
										char gateType, struct gateOrWire **circuit,
										unsigned char *R)
{
	struct gateOrWire *toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));

	toReturn -> G_ID = idNum;
	toReturn -> outputWire = (struct wire *) calloc(1, sizeof(struct wire));
	toReturn -> outputWire -> wirePerm = getPermutation();
	toReturn -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));

	// toReturn -> outputWire -> outputGarbleKeys = generateGarbleKeyPair(toReturn -> outputWire -> wirePerm);
	toReturn -> outputWire -> outputGarbleKeys = generateFreeXORPair(toReturn -> outputWire -> wirePerm, R);

	if('X' == gateType)
	{
		toReturn -> outputWire -> wireMask ^= 0xF0;
	}

	toReturn -> gatePayload = processGateRTL(numInputWires, inputIDs, gateType);
	encWholeOutTable(toReturn, circuit);

	return toReturn;
}


// Take a line of the input file and make a gateOrWire struct from it.
struct gateOrWire *processGateLineRTL(char *line, struct gateOrWire **circuit, unsigned char *R)
{
	int strIndex = 0, idNum, i;
	int numInputWires, purposelessNumber;
	int *inputIDs;

	numInputWires = getIntFromString(line, strIndex);

	// As of yet unsure of the purpose of this number. Ask Nigel? Otherwise we just ignore it.
	purposelessNumber = getIntFromString(line, strIndex);

	inputIDs = (int*) calloc(numInputWires, sizeof(int));
	for(i = 0; i < numInputWires; i ++)
	{
		inputIDs[i] = getIntFromString(line, strIndex);
	}

	idNum = getIntFromString(line, strIndex);

	return processGateOrWireRTL(idNum, inputIDs, numInputWires, line[strIndex], circuit, R);
}


// Initialises an input wire, needed because input wires don't feature in the input file.
// Therefore they need to initialised separately.
struct gateOrWire *initialiseInputWire(int idNum, unsigned char owner, unsigned char *R)
{
	struct gateOrWire *toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));

	toReturn -> G_ID = idNum;
	toReturn -> outputWire = (struct wire *) calloc(1, sizeof(struct wire));
	toReturn -> outputWire -> wirePerm = getPermutation();
	toReturn -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));

	// toReturn -> outputWire -> outputGarbleKeys = generateGarbleKeyPair(toReturn -> outputWire -> wirePerm);
	toReturn -> outputWire -> outputGarbleKeys = generateFreeXORPair(toReturn -> outputWire -> wirePerm, R);

	toReturn -> gatePayload = NULL;
	toReturn -> outputWire -> wireMask = 0x01;
	toReturn -> outputWire -> wireOwner = owner;

	return toReturn;
}


// FOR NOW we assume party 1 is building the circuit.
struct gateOrWire **initialiseAllInputs(int numGates, int numInputs1, int numInputs2, int **execOrder, unsigned char *R)
{
	struct gateOrWire **circuit = (struct gateOrWire**) calloc(numGates, sizeof(struct gateOrWire*));
	int i;

	for(i = 0; i < numInputs1; i ++)
	{
		circuit[i] = initialiseInputWire(i, 0xFF, R);
		(*execOrder)[i] = i;
	}

	for(i = numInputs2; i < numInputs1 + numInputs2; i ++)
	{
		circuit[i] = initialiseInputWire(i, 0x00, R);
		(*execOrder)[i] = i;
	}

	return circuit;
}


// Create a circuit given a file in RTL format.
struct Circuit *readInCircuitRTL(char* filepath)
{
	FILE *file = fopen ( filepath, "r" );
	char line [ 512 ]; // Or other suitable maximum line size
	int numInputs1, numInputs2, numOutputs, gateIndex = 0;
	struct gateOrWire *tempGateOrWire;
	struct gateOrWire **gatesList;
	struct Circuit *outputCircuit = (struct Circuit*) calloc(1, sizeof(struct Circuit));
	int i, execIndex, *execOrder;
	unsigned char *R = generateRandBytes(16, 17);

	if ( file != NULL )
	{
		if(NULL != fgets(line, sizeof(line), file))
			sscanf( line, "%d %d", &numInputs1, &(outputCircuit -> numGates) );
		else
			return NULL;

		if(NULL != fgets(line, sizeof(line), file))
			sscanf(line, "%d %d\t%d", &numInputs1, &numInputs2, &(outputCircuit -> numOutputs));
		else
			return NULL;

		if(NULL == fgets(line, sizeof(line), file))
			return NULL;

		execOrder = (int*) calloc(outputCircuit -> numGates, sizeof(int));
		gatesList = initialiseAllInputs( outputCircuit -> numGates, numInputs1, numInputs2, &execOrder, R );
		execIndex = numInputs1 + numInputs2;

		while ( fgets(line, sizeof(line), file) != NULL ) // Read a line
		{
			tempGateOrWire = processGateLineRTL(line, gatesList, R);
			if( NULL != tempGateOrWire )
			{
				gateIndex = tempGateOrWire -> G_ID;
				*(gatesList + gateIndex) = tempGateOrWire;
				*(execOrder + execIndex) = gateIndex;
				execIndex++;
			}
		}

		for(i = 0; i < outputCircuit -> numOutputs; i ++)
		{
			gateIndex = outputCircuit -> numGates - i - 1;
			gatesList[gateIndex] -> outputWire -> wireMask = 0x02;
		}
		fclose ( file );
	}

	outputCircuit -> gates = gatesList;
	outputCircuit -> execOrder = execOrder;

	free(R);

	return outputCircuit;
}