// RTL means the function deals with the Smart/Tillich style input.
struct gate *processGate_FromRaw(int numInputWires, int *inputIDs, int *rawOutputTable, char gateType)
{
	struct gate *toReturn = (struct gate*) calloc(1, sizeof(struct gate));
	int i, outputTableSize = 1;

	toReturn -> numInputs = numInputWires;
	toReturn -> inputIDs = inputIDs;


	for(i = 0; i < toReturn -> numInputs; i ++)
		outputTableSize *= 2;

	toReturn -> outputTableSize = outputTableSize;
	toReturn -> rawOutputTable = rawOutputTable;

	toReturn -> encOutputTable = createOutputTable(toReturn);


	return toReturn;
}


// Process a gateOrWire struct given the data.
struct gateOrWire *processGateOrWire_FromRaw(randctx *ctx, struct RawGate *rawGate, struct gateOrWire **circuit, unsigned char *R, int numInputs1)
{
	struct gateOrWire *toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));
	unsigned char permC = 0x00, usingBuilderInput = 0xF0;
	int inputID, i;

	toReturn -> G_ID = rawGate -> G_ID;
	toReturn -> outputWire = (struct wire *) calloc(1, sizeof(struct wire));
	toReturn -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));

	toReturn -> gatePayload = processGate_FromRaw(rawGate -> numInputs, rawGate -> inputIDs, rawGate -> rawOutputTable, rawGate -> gateType);

	if('X' == rawGate -> gateType)
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
		toReturn -> outputWire -> wirePerm = getIsaacPermutation(ctx);
	}

	toReturn -> outputWire -> outputGarbleKeys = genFreeXORPair(toReturn, R, circuit);

	encWholeOutTable(toReturn, circuit);

	return toReturn;
}


// Process a gateOrWire struct given the data.
struct gateOrWire *processGateOrWire_FromRaw_ConsistentOutput(randctx *ctx, struct RawGate *rawGate, struct gateOrWire **circuit,
															unsigned char *b0, unsigned char *b1, int numInputs1)
{
	struct gateOrWire *toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));
	unsigned char permC = 0x00, usingBuilderInput = 0xF0;
	int inputID, i;

	toReturn -> G_ID = rawGate -> G_ID;
	toReturn -> outputWire = (struct wire *) calloc(1, sizeof(struct wire));
	toReturn -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));

	toReturn -> gatePayload = processGate_FromRaw(rawGate -> numInputs, rawGate -> inputIDs, rawGate -> rawOutputTable, rawGate -> gateType);

	if('X' == rawGate -> gateType)
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
		toReturn -> outputWire -> wirePerm = getIsaacPermutation(ctx);
	}


	toReturn -> outputWire -> outputGarbleKeys = (struct bitsGarbleKeys*) calloc(1, sizeof(struct bitsGarbleKeys));
	toReturn -> outputWire -> outputGarbleKeys -> key0 = (unsigned char *) calloc(17, sizeof(unsigned char));
	toReturn -> outputWire -> outputGarbleKeys -> key1 = (unsigned char *) calloc(17, sizeof(unsigned char));

	memcpy(toReturn -> outputWire -> outputGarbleKeys -> key0, b0, 16);
	memcpy(toReturn -> outputWire -> outputGarbleKeys -> key1, b1, 16);

	toReturn -> outputWire -> outputGarbleKeys -> key0[16] = 0x00 ^ (0x01 & toReturn -> outputWire -> wirePerm);
	toReturn -> outputWire -> outputGarbleKeys -> key1[16] = 0x01 ^ (0x01 & toReturn -> outputWire -> wirePerm);


	encWholeOutTable(toReturn, circuit);

	return toReturn;
}


// Take a line of the input file and make a gateOrWire struct from it.
struct gateOrWire *processGateLine_FromRaw(randctx *ctx, struct RawGate *rawGate, struct gateOrWire **circuit,
											unsigned char *R, int idOffset, int numInputs1)
{
	int strIndex = 0, idNum, i;

	idNum = rawGate -> G_ID;

	return processGateOrWire_FromRaw(ctx, rawGate, circuit, R, numInputs1);
}


// Initialises an input wire, needed because input wires don't feature in the input file.
// Therefore they need to initialised separately.
struct gateOrWire *initInputWire_FromRaw(randctx *ctx, int idNum, unsigned char owner, unsigned char *R)
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



// Initialises an input wire, needed because input wires don't feature in the input file.
// Therefore they need to initialised separately.
struct gateOrWire *initInputWire_FromRaw_ConsistentInput(randctx *ctx, int idNum, unsigned char owner, unsigned char *R,
													struct eccPoint **consistentInputs,
													int j, struct eccParams *params)
{
	struct gateOrWire *toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));
	struct bitsGarbleKeys *tempOutput;

	toReturn -> G_ID = idNum;
	toReturn -> outputWire = (struct wire *) calloc(1, sizeof(struct wire));
	toReturn -> outputWire -> wirePerm = getIsaacPermutation(ctx);
	toReturn -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));


	tempOutput = (struct bitsGarbleKeys*) calloc(1, sizeof(struct bitsGarbleKeys));

	tempOutput -> key0 = hashECC_Point(consistentInputs[2 * idNum], 16);
	tempOutput -> key1 = hashECC_Point(consistentInputs[2 * idNum + 1], 16);


	toReturn -> outputWire -> outputGarbleKeys = tempOutput;

	toReturn -> gatePayload = NULL;
	toReturn -> outputWire -> wireMask = 0x01;
	toReturn -> outputWire -> wireOwner = owner;

	return toReturn;
}


// We assume party 1 is building the circuit.
struct gateOrWire **initAllInputs_FromRaw_ConsistentInput(randctx *ctx, struct RawCircuit *rawInputCircuit, unsigned char *R,
													struct eccPoint **consistentInputs, int j, struct eccParams *params)
{
	struct gateOrWire **gates = (struct gateOrWire**) calloc(rawInputCircuit -> numGates, sizeof(struct gateOrWire*));
	int i, numInputs = rawInputCircuit -> numInputs_P1 + rawInputCircuit -> numInputs_P2;

	for(i = 0; i < rawInputCircuit -> numInputs_P1; i ++)
	{
		gates[i] = initInputWire_FromRaw_ConsistentInput(ctx, i, 0xFF, R, consistentInputs, j, params);
	}

	for(; i < numInputs; i ++)
	{
		gates[i] = initInputWire_FromRaw(ctx, i, 0x00, R);
	}


	return gates;
}


// We assume party 1 is building the circuit.
struct gateOrWire **initAllInputs_FromRaw(randctx *ctx, struct RawCircuit *rawInputCircuit, unsigned char *R)
{
	struct gateOrWire **gates = (struct gateOrWire**) calloc(rawInputCircuit -> numGates, sizeof(struct gateOrWire*));
	int i, numInputs = rawInputCircuit -> numInputs_P1 + rawInputCircuit -> numInputs_P2;

	for(i = 0; i < rawInputCircuit -> numInputs_P1; i ++)
	{
		gates[i] = initInputWire_FromRaw(ctx, i, 0xFF, R);
	}

	for(; i < numInputs; i ++)
	{
		gates[i] = initInputWire_FromRaw(ctx, i, 0x00, R);
	}


	return gates;
}


// Create a circuit given a file in RTL format.
struct Circuit *readInCircuit_FromRaw_ConsistentInput(randctx *ctx, struct RawCircuit *rawInputCircuit,
													struct eccPoint **consistentInputs,
													int j, struct eccParams *params)
{
	struct gateOrWire *tempGateOrWire;
	struct gateOrWire **gatesList;

	struct Circuit *outputCircuit = (struct Circuit*) calloc(1, sizeof(struct Circuit));
	int i, gateIndex = 0;

	unsigned char *R = generateIsaacRandBytes(ctx, 16, 17);


	outputCircuit -> numGates = rawInputCircuit -> numGates;

	outputCircuit -> checkFlag = 0x00;
	outputCircuit -> numInputsBuilder = rawInputCircuit -> numInputs_P1;
	outputCircuit -> numInputsExecutor = rawInputCircuit -> numInputs_P2;
	outputCircuit -> numInputs = rawInputCircuit -> numInputs_P1 + rawInputCircuit -> numInputs_P2;
	outputCircuit -> numOutputs = rawInputCircuit -> numOutputs;


	gatesList = initAllInputs_FromRaw_ConsistentInput( ctx, rawInputCircuit, R, consistentInputs, j, params );

	for(i = outputCircuit -> numInputs; i < outputCircuit -> numGates; i ++)
	{
		gateIndex = rawInputCircuit -> execOrder[i];

		tempGateOrWire = processGateLine_FromRaw(ctx, rawInputCircuit -> gates[gateIndex], gatesList, R, 0, outputCircuit -> numInputsBuilder);

		if( NULL != tempGateOrWire )
		{
			*(gatesList + gateIndex) = tempGateOrWire;
		}
	}

	for(i = 0; i < outputCircuit -> numOutputs; i ++)
	{
		gateIndex = outputCircuit -> numGates - i - 1;
		gatesList[gateIndex] -> outputWire -> wireMask = 0x02;
	}

	outputCircuit -> gates = gatesList;
	outputCircuit -> execOrder = rawInputCircuit -> execOrder;

	free(R);

	return outputCircuit;
}


// Create a circuit given a file in RTL format.
struct Circuit *readInCircuit_FromRaw_ConsistentInputOutput(randctx *ctx, struct RawCircuit *rawInputCircuit,
													struct eccPoint **consistentInputs, unsigned char **b0List, unsigned char **b1List,
													int j, struct eccParams *params)
{
	struct gateOrWire *tempGateOrWire;
	struct gateOrWire **gatesList;

	struct Circuit *outputCircuit = (struct Circuit*) calloc(1, sizeof(struct Circuit));
	int i, k = 0, gateIndex = 0;

	unsigned char *R = generateIsaacRandBytes(ctx, 16, 17);


	outputCircuit -> numGates = rawInputCircuit -> numGates;

	outputCircuit -> checkFlag = 0x00;
	outputCircuit -> numInputsBuilder = rawInputCircuit -> numInputs_P1;
	outputCircuit -> numInputsExecutor = rawInputCircuit -> numInputs_P2;
	outputCircuit -> numInputs = rawInputCircuit -> numInputs_P1 + rawInputCircuit -> numInputs_P2;
	outputCircuit -> numOutputs = rawInputCircuit -> numOutputs;


	gatesList = initAllInputs_FromRaw_ConsistentInput( ctx, rawInputCircuit, R, consistentInputs, j, params );

	for(i = outputCircuit -> numInputs; i < outputCircuit -> numGates; i ++)
	{
		gateIndex = rawInputCircuit -> execOrder[i];

		if(gateIndex < outputCircuit -> numGates - outputCircuit -> numOutputs)
		{
			tempGateOrWire = processGateLine_FromRaw(ctx, rawInputCircuit -> gates[gateIndex], gatesList, R, 0, outputCircuit -> numInputsBuilder);
		}
		else
		{
			tempGateOrWire = processGateOrWire_FromRaw_ConsistentOutput(ctx, rawInputCircuit -> gates[gateIndex], gatesList, b0List[k], b1List[k], outputCircuit -> numInputsBuilder);
			k ++;
		}

		if( NULL != tempGateOrWire )
		{
			*(gatesList + gateIndex) = tempGateOrWire;
		}
	}

	for(i = 0; i < outputCircuit -> numOutputs; i ++)
	{
		gateIndex = outputCircuit -> numGates - i - 1;
		gatesList[gateIndex] -> outputWire -> wireMask = 0x02;
	}

	outputCircuit -> gates = gatesList;
	outputCircuit -> execOrder = rawInputCircuit -> execOrder;

	free(R);

	return outputCircuit;
}

/*
struct Circuit *readInCircuit_FromRaw(randctx *ctx, struct RawCircuit *rawInputCircuit)
{
	struct gateOrWire *tempGateOrWire;
	struct gateOrWire **gatesList;

	struct Circuit *outputCircuit = (struct Circuit*) calloc(1, sizeof(struct Circuit));
	int i, gateIndex = 0;

	unsigned char *R = generateIsaacRandBytes(ctx, 16, 17);


	outputCircuit -> numGates = rawInputCircuit -> numGates;

	outputCircuit -> checkFlag = 0x00;
	outputCircuit -> numInputsBuilder = rawInputCircuit -> numInputs_P1;
	outputCircuit -> numInputsExecutor = rawInputCircuit -> numInputs_P1;
	outputCircuit -> numInputs = rawInputCircuit -> numInputs_P1 + rawInputCircuit -> numInputs_P2;
	outputCircuit -> numOutputs = rawInputCircuit -> numOutputs;


	gatesList = initAllInputs_FromRaw( ctx, rawInputCircuit, R );

	for(i = outputCircuit -> numInputs; i < outputCircuit -> numGates; i ++)
	{
		gateIndex = rawInputCircuit -> execOrder[i];

		tempGateOrWire = processGateLine_FromRaw(ctx, rawInputCircuit -> gates[gateIndex], gatesList, R, 0, outputCircuit -> numInputsBuilder);

		if( NULL != tempGateOrWire )
		{
			*(gatesList + gateIndex) = tempGateOrWire;
		}
	}

	for(i = 0; i < outputCircuit -> numOutputs; i ++)
	{
		gateIndex = outputCircuit -> numGates - i - 1;
		gatesList[gateIndex] -> outputWire -> wireMask = 0x02;
	}

	outputCircuit -> gates = gatesList;
	outputCircuit -> execOrder = rawInputCircuit -> execOrder;

	free(R);

	return outputCircuit;
}
*/



struct Circuit *readInCircuit_FromRaw_Seeded_ConsistentInput(randctx *ctx, struct RawCircuit *rawInputCircuit,
															struct eccPoint **consistentInputs,
															int j, struct eccParams *params)
{
	struct Circuit *toReturn;

	toReturn = readInCircuit_FromRaw_ConsistentInput(ctx, rawInputCircuit, consistentInputs, j, params);

	return toReturn;
}


struct Circuit *readInCircuit_FromRaw_Seeded_ConsistentInputOutput(randctx *ctx, struct RawCircuit *rawInputCircuit,
															struct eccPoint **consistentInputs, unsigned char **b0List, unsigned char **b1List,
															int j, struct eccParams *params)
{
	struct Circuit *toReturn;

	toReturn = readInCircuit_FromRaw_ConsistentInputOutput(ctx, rawInputCircuit, consistentInputs, b0List, b1List, j, params);

	return toReturn;
}