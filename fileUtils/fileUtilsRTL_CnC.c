// RTL means the function deals with the Smart/Tillich style input.
struct gate *processGateRTL_CnC(int numInputWires, int *inputIDs, char gateType)
{
	struct gate *toReturn = (struct gate*) calloc(1, sizeof(struct gate));
	int i, outputTableSize = 1;

	toReturn -> numInputs = numInputWires;
	toReturn -> inputIDs = (int*) calloc(2, sizeof(int));

	toReturn -> inputIDs[0] = inputIDs[0];
	toReturn -> inputIDs[1] = inputIDs[1];

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
struct gateOrWire *processGateOrWireRTL_CnC(int idNum, int *inputIDs, int numInputWires,
										char gateType, struct gateOrWire **circuit,
										unsigned char *R, int numInputs1)
{
	struct gateOrWire *toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));
	unsigned char permC = 0x00;
	unsigned char usingBuilderInput = 0xF0;
	int inputID, i;

	toReturn -> G_ID = idNum;
	toReturn -> outputWire = (struct wire *) calloc(1, sizeof(struct wire));
	toReturn -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));

	// toReturn -> outputWire -> outputGarbleKeys = generateGarbleKeyPair(toReturn -> outputWire -> wirePerm);
	toReturn -> gatePayload = processGateRTL_CnC(numInputWires, inputIDs, gateType);

	if('X' == gateType)
	{
		for(i = 0; i < toReturn -> gatePayload -> numInputs; i ++)
		{
			inputID = toReturn -> gatePayload -> inputIDs[i];
			permC ^= circuit[inputID] -> outputWire -> wirePerm;
			if(inputID < numInputs1)
			{
				usingBuilderInput &= 0xE0;
			}		
		}

		toReturn -> outputWire -> wireMask ^= usingBuilderInput;
		toReturn -> outputWire -> wirePerm = permC;
	}
	else
	{
		toReturn -> outputWire -> wirePerm = getPermutation();
	}

	// toReturn -> outputWire -> outputGarbleKeys = genFreeXORPair(toReturn, R, circuit);

	// encWholeOutTable(toReturn, circuit);

	return toReturn;
}


// Take a line of the input file and make a gateOrWire struct from it.
struct gateOrWire *processGateLineRTL_CnC(char *line, struct gateOrWire **circuit, unsigned char *R, int idOffset, int numInputs1)
{
	int strIndex = 0, idNum, i;
	int numInputWires, purposelessNumber;
	int *inputIDs;

	numInputWires = getIntFromString(line, strIndex);

	// As of yet unsure of the purpose of this number. Ask Nigel? Otherwise we just ignore it.
	// Is it the number of output wires?
	purposelessNumber = getIntFromString(line, strIndex);

	inputIDs = (int*) calloc(numInputWires, sizeof(int));
	for(i = 0; i < numInputWires; i ++)
	{
		idNum = getIntFromString(line, strIndex);
		if(idNum >= numInputs1)
		{
			inputIDs[i] = idNum + idOffset;
		}
		else
		{
			inputIDs[i] = idNum;
		}
	}

	idNum = getIntFromString(line, strIndex) + idOffset;

	return processGateOrWireRTL_CnC(idNum, inputIDs, numInputWires, line[strIndex], circuit, R, numInputs1);
}


// Initialises an input wire, needed because input wires don't feature in the input file.
// Therefore they need to initialised separately.
struct gateOrWire *initialiseInputWire_CnC(int idNum, unsigned char owner, unsigned char *R)
{
	struct gateOrWire *toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));

	toReturn -> G_ID = idNum;
	toReturn -> outputWire = (struct wire *) calloc(1, sizeof(struct wire));
	toReturn -> outputWire -> wirePerm = getPermutation();
	toReturn -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));

	if(0x00 == owner)
	{
		toReturn -> outputWire -> outputGarbleKeys = genFreeXORPairInput(toReturn -> outputWire -> wirePerm, R);
	}

	toReturn -> gatePayload = NULL;
	toReturn -> outputWire -> wireMask = 0x01;
	toReturn -> outputWire -> wireOwner = owner;

	return toReturn;
}


// We assume party 1 is building the circuit.
struct gateOrWire **initialiseAllInputs_CnC(int numGates, int numInputs1, int numInputs2, unsigned char *R)
{
	struct gateOrWire **circuit = (struct gateOrWire**) calloc(numGates, sizeof(struct gateOrWire*));
	int i;

	for(i = 0; i < numInputs1; i ++)
	{
		circuit[i] = initialiseInputWire_CnC(i, 0xFF, R);
	}

	for(i = numInputs2; i < numInputs1 + numInputs2; i ++)
	{
		circuit[i] = initialiseInputWire_CnC(i, 0x00, R);
	}


	return circuit;
}



// Create a circuit given a file in RTL format.
struct Circuit *readInCircuitRTL_CnC(char* filepath, unsigned char *R)
{
	FILE *file = fopen ( filepath, "r" );
	char line [ 512 ]; // Or other suitable maximum line size
	int numInputs1, numInputs2, numOutputs, gateIndex = 0;
	struct gateOrWire *tempGateOrWire;
	struct gateOrWire **gatesList;

	struct Circuit *outputCircuit = (struct Circuit*) calloc(1, sizeof(struct Circuit));
	int i, execIndex;
	// unsigned char *R = generateRandBytes(16, 17);


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

		outputCircuit -> checkFlag = 0x00;
		outputCircuit -> numInputsBuilder = numInputs1;
		outputCircuit -> numInputsExecutor = numInputs2;
		outputCircuit -> securityParam = 1;

		outputCircuit -> execOrder = (int*) calloc(outputCircuit -> numGates, sizeof(int));
		gatesList = initialiseAllInputs_CnC( outputCircuit -> numGates, numInputs1, numInputs2, R );
		execIndex = numInputs1 + numInputs2;

		for(i = 0; i < execIndex; i ++)
			outputCircuit -> execOrder[i] = i;

		while ( fgets(line, sizeof(line), file) != NULL ) // Read a line
		{
			tempGateOrWire = processGateLineRTL_CnC(line, gatesList, R, 0, numInputs1);
			if( NULL != tempGateOrWire )
			{
				gateIndex = tempGateOrWire -> G_ID;
				*(gatesList + gateIndex) = tempGateOrWire;
				outputCircuit -> execOrder[execIndex] = gateIndex;
				execIndex++;
			}
		}

		for(i = 0; i < outputCircuit -> numOutputs; i ++)
		{
			gateIndex = outputCircuit -> numGates - i - 1;
			gatesList[gateIndex] -> outputWire -> wireMask |= 0x02;
		}
		fclose ( file );
	}

	outputCircuit -> gates = gatesList;
	// outputCircuit -> execOrder = execOrder;

	free(R);

	return outputCircuit;
}