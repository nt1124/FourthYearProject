
// Function for use in testing, will set all inputs to zero and
// initialise accordingly.
void zeroAllInputs(struct gateOrWire **inputCircuit, int numGates)
{
	struct wire* outputWire;
	int i;

	for(i = 0; i < numGates; i ++)
	{
		if( 0x01 != (0x0F & inputCircuit[i] -> outputWire -> wireMask) )
			break;
		outputWire = inputCircuit[i] -> outputWire;

		outputWire -> wireOwner = 0xFF;
		outputWire -> wirePermedValue = outputWire -> outputGarbleKeys -> key0[16];
		memcpy(outputWire -> wireOutputKey, outputWire -> outputGarbleKeys -> key0, 16);
		// printf("%d - %02X\n", i, (outputWire -> wirePermedValue ^ (outputWire -> wirePerm & 0x01)));
	}
}



// RTL means the function deals with the Smart/Tillich style input.
struct gate *processGateRTL(int numInputWires, int *inputIDs, char gateType)
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
struct gateOrWire *processGateOrWireRTL(randctx *globalIsaacContext, int idNum, int *inputIDs, int numInputWires,
										char gateType, struct gateOrWire **circuit,
										unsigned char *R, int numInputs1)
{
	struct gateOrWire *toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));
	unsigned char permC = 0x00, usingBuilderInput = 0xF0;
	int inputID, i;

	toReturn -> G_ID = idNum;
	toReturn -> outputWire = (struct wire *) calloc(1, sizeof(struct wire));
	toReturn -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));

	toReturn -> gatePayload = processGateRTL(numInputWires, inputIDs, gateType);

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
		toReturn -> outputWire -> wirePerm = getIsaacPermutation(globalIsaacContext);
	}

	toReturn -> outputWire -> outputGarbleKeys = genFreeXORPair(toReturn, R, circuit);

	encWholeOutTable(toReturn, circuit);

	return toReturn;
}


// Take a line of the input file and make a gateOrWire struct from it.
struct gateOrWire *processGateLineRTL(randctx *globalIsaacContext, char *line, struct gateOrWire **circuit, unsigned char *R, int idOffset, int numInputs1)
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

	return processGateOrWireRTL(globalIsaacContext, idNum, inputIDs, numInputWires, line[strIndex], circuit, R, numInputs1);
}


// Initialises an input wire, needed because input wires don't feature in the input file.
// Therefore they need to initialised separately.
struct gateOrWire *initialiseInputWire(randctx *ctx, int idNum, unsigned char owner, unsigned char *R)
{
	struct gateOrWire *toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));

	toReturn -> G_ID = idNum;
	toReturn -> outputWire = (struct wire *) calloc(1, sizeof(struct wire));
	toReturn -> outputWire -> wirePerm = getIsaacPermutation(ctx);
	toReturn -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));

	toReturn -> outputWire -> outputGarbleKeys = genFreeXORPairInput(ctx, toReturn -> outputWire -> wirePerm, R);

	toReturn -> gatePayload = NULL;
	toReturn -> outputWire -> wireMask = 0x01;
	toReturn -> outputWire -> wireOwner = owner;

	return toReturn;
}


// We assume party 1 is building the circuit.
struct gateOrWire **initialiseAllInputs(randctx *ctx, int numGates, int numInputs1, int numInputs2, int **execOrder, unsigned char *R)
{
	struct gateOrWire **circuit = (struct gateOrWire**) calloc(numGates, sizeof(struct gateOrWire*));
	int i;

	for(i = 0; i < numInputs1; i ++)
	{
		circuit[i] = initialiseInputWire(ctx, i, 0xFF, R);
		(*execOrder)[i] = i;
	}

	for(i = numInputs2; i < numInputs1 + numInputs2; i ++)
	{
		circuit[i] = initialiseInputWire(ctx, i, 0x00, R);
		(*execOrder)[i] = i;
	}


	return circuit;
}


// Create a circuit given a file in RTL format.
struct Circuit *readInCircuitRTL(char *filepath, randctx *globalIsaacContext)
{
	FILE *file = fopen ( filepath, "r" );
	char line [ 512 ]; // Or other suitable maximum line size
	int numInputs1, numInputs2, numOutputs, gateIndex = 0;
	struct gateOrWire *tempGateOrWire;
	struct gateOrWire **gatesList;
	struct Circuit *outputCircuit = (struct Circuit*) calloc(1, sizeof(struct Circuit));
	int i, execIndex, *execOrder;
	unsigned char *R = generateIsaacRandBytes(globalIsaacContext, 16, 17);


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

		execOrder = (int*) calloc(outputCircuit -> numGates, sizeof(int));
		gatesList = initialiseAllInputs( globalIsaacContext, outputCircuit -> numGates, numInputs1, numInputs2, &execOrder, R );
		execIndex = numInputs1 + numInputs2;

		while ( fgets(line, sizeof(line), file) != NULL ) // Read a line
		{
			tempGateOrWire = processGateLineRTL(globalIsaacContext, line, gatesList, R, 0, numInputs1);
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

