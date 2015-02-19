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
struct gateOrWire *processGateOrWire_FromRaw(struct RawGate *rawGate, struct gateOrWire **circuit,
										unsigned char *R, int numInputs1)
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
		toReturn -> outputWire -> wirePerm = getPermutation();
	}

	toReturn -> outputWire -> outputGarbleKeys = genFreeXORPair(toReturn, R, circuit);

	encWholeOutTable(toReturn, circuit);

	return toReturn;
}


// Take a line of the input file and make a gateOrWire struct from it.
struct gateOrWire *processGateLine_FromRaw(struct RawGate *rawGate, struct gateOrWire **circuit,
											unsigned char *R, int idOffset, int numInputs1)
{
	int strIndex = 0, idNum, i;

	idNum = rawGate -> G_ID;

	return processGateOrWire_FromRaw(rawGate, circuit, R, numInputs1);
}


// Initialises an input wire, needed because input wires don't feature in the input file.
// Therefore they need to initialised separately.
struct gateOrWire *initInputWire_FromRaw(int idNum, unsigned char owner, unsigned char *R)
{
	struct gateOrWire *toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));

	toReturn -> G_ID = idNum;
	toReturn -> outputWire = (struct wire *) calloc(1, sizeof(struct wire));
	toReturn -> outputWire -> wirePerm = getPermutation();
	toReturn -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));

	toReturn -> outputWire -> outputGarbleKeys = genFreeXORPairInput(toReturn -> outputWire -> wirePerm, R);

	toReturn -> gatePayload = NULL;
	toReturn -> outputWire -> wireMask = 0x01;
	toReturn -> outputWire -> wireOwner = owner;

	return toReturn;
}



// Initialises an input wire, needed because input wires don't feature in the input file.
// Therefore they need to initialised separately.
struct gateOrWire *initInputWire_FromRaw_ConsistentInput(int idNum, unsigned char owner, unsigned char *R,
													mpz_t secret_input,
													struct public_builderPRS_Keys *public_inputs,
													int j, struct DDH_Group *group)
{
	struct gateOrWire *toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));
	struct bitsGarbleKeys *tempOutput;

	toReturn -> G_ID = idNum;
	toReturn -> outputWire = (struct wire *) calloc(1, sizeof(struct wire));
	toReturn -> outputWire -> wirePerm = getPermutation();
	toReturn -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));

	// toReturn -> outputWire -> outputGarbleKeys = genFreeXORPairInput(toReturn -> outputWire -> wirePerm, R);

	tempOutput = (struct bitsGarbleKeys*) calloc(1, sizeof(struct bitsGarbleKeys));
	/*
	tempOutput -> key0 = compute_Key_b_Input_i_Circuit_j(secret_inputs, public_inputs, group, idNum, j,
														0x00, toReturn -> outputWire -> wirePerm);
	tempOutput -> key1 = compute_Key_b_Input_i_Circuit_j(secret_inputs, public_inputs, group, idNum, j,
														0x01, toReturn -> outputWire -> wirePerm);
	*/
	tempOutput -> key0 = compute_Key_b_Input_i_Circuit_j(secret_input, public_inputs, group, idNum,	0x00);
	tempOutput -> key1 = compute_Key_b_Input_i_Circuit_j(secret_input, public_inputs, group, idNum,	0x01);

	toReturn -> outputWire -> outputGarbleKeys = tempOutput;

	toReturn -> gatePayload = NULL;
	toReturn -> outputWire -> wireMask = 0x01;
	toReturn -> outputWire -> wireOwner = owner;

	return toReturn;
}


// We assume party 1 is building the circuit.
struct gateOrWire **initAllInputs_FromRaw_ConsistentInput(struct RawCircuit *rawInputCircuit, unsigned char *R,
													mpz_t secret_input,
													struct public_builderPRS_Keys *public_inputs,
													int j, struct DDH_Group *group)
{
	struct gateOrWire **gates = (struct gateOrWire**) calloc(rawInputCircuit -> numGates, sizeof(struct gateOrWire*));
	int i, numInputs = rawInputCircuit -> numInputsBuilder + rawInputCircuit -> numInputsExecutor;

	for(i = 0; i < rawInputCircuit -> numInputsBuilder; i ++)
	{
		gates[i] = initInputWire_FromRaw_ConsistentInput(i, 0xFF, R, secret_input, public_inputs, j, group);
	}

	for(; i < numInputs; i ++)
	{
		gates[i] = initInputWire_FromRaw(i, 0x00, R);
	}


	return gates;
}


// We assume party 1 is building the circuit.
struct gateOrWire **initAllInputs_FromRaw(struct RawCircuit *rawInputCircuit, unsigned char *R)
{
	struct gateOrWire **gates = (struct gateOrWire**) calloc(rawInputCircuit -> numGates, sizeof(struct gateOrWire*));
	int i, numInputs = rawInputCircuit -> numInputsBuilder + rawInputCircuit -> numInputsExecutor;

	for(i = 0; i < rawInputCircuit -> numInputsBuilder; i ++)
	{
		gates[i] = initInputWire_FromRaw(i, 0xFF, R);
	}

	for(; i < numInputs; i ++)
	{
		gates[i] = initInputWire_FromRaw(i, 0x00, R);
	}


	return gates;
}


// Create a circuit given a file in RTL format.
struct Circuit *readInCircuit_FromRaw_ConsistentInput(struct RawCircuit *rawInputCircuit,
													mpz_t secret_input,
													struct public_builderPRS_Keys *public_inputs,
													int j, struct DDH_Group *group)
{
struct gateOrWire *tempGateOrWire;
	struct gateOrWire **gatesList;

	struct Circuit *outputCircuit = (struct Circuit*) calloc(1, sizeof(struct Circuit));
	int i, gateIndex = 0;

	unsigned char *R = generateRandBytes(16, 17);


	outputCircuit -> numGates = rawInputCircuit -> numGates;

	outputCircuit -> checkFlag = 0x00;
	outputCircuit -> numInputsBuilder = rawInputCircuit -> numInputsBuilder;
	outputCircuit -> numInputsExecutor = rawInputCircuit -> numInputsBuilder;
	outputCircuit -> numInputs = rawInputCircuit -> numInputsBuilder + rawInputCircuit -> numInputsExecutor;
	outputCircuit -> numOutputs = rawInputCircuit -> numOutputs;


	gatesList = initAllInputs_FromRaw_ConsistentInput( rawInputCircuit, R, secret_input, public_inputs, j, group );

	for(i = outputCircuit -> numInputs; i < outputCircuit -> numGates; i ++)
	{
		gateIndex = rawInputCircuit -> execOrder[i];

		tempGateOrWire = processGateLine_FromRaw(rawInputCircuit -> gates[gateIndex], gatesList, R, 0, outputCircuit -> numInputsBuilder);

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


struct Circuit *readInCircuit_FromRaw(struct RawCircuit *rawInputCircuit)
{
	struct gateOrWire *tempGateOrWire;
	struct gateOrWire **gatesList;

	struct Circuit *outputCircuit = (struct Circuit*) calloc(1, sizeof(struct Circuit));
	int i, gateIndex = 0;

	unsigned char *R = generateRandBytes(16, 17);


	outputCircuit -> numGates = rawInputCircuit -> numGates;

	outputCircuit -> checkFlag = 0x00;
	outputCircuit -> numInputsBuilder = rawInputCircuit -> numInputsBuilder;
	outputCircuit -> numInputsExecutor = rawInputCircuit -> numInputsBuilder;
	outputCircuit -> numInputs = rawInputCircuit -> numInputsBuilder + rawInputCircuit -> numInputsExecutor;
	outputCircuit -> numOutputs = rawInputCircuit -> numOutputs;


	gatesList = initAllInputs_FromRaw( rawInputCircuit, R );

	for(i = outputCircuit -> numInputs; i < outputCircuit -> numGates; i ++)
	{
		gateIndex = rawInputCircuit -> execOrder[i];

		tempGateOrWire = processGateLine_FromRaw(rawInputCircuit -> gates[gateIndex], gatesList, R, 0, outputCircuit -> numInputsBuilder);

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

struct Circuit *readInCircuit_FromRaw_Seeded(struct RawCircuit *rawInputCircuit, unsigned int seed)
{
	struct Circuit *toReturn;
	srand(seed);

	toReturn = readInCircuit_FromRaw(rawInputCircuit);
	toReturn -> seed = seed;

	return toReturn;
}

struct Circuit *readInCircuit_FromRaw_Seeded_ConsistentInput(struct RawCircuit *rawInputCircuit, unsigned int seed, 
															mpz_t secret_input,
															struct public_builderPRS_Keys *public_inputs,
															int j, struct DDH_Group *group)
{
	struct Circuit *toReturn;
	srand(seed);

	toReturn = readInCircuit_FromRaw_ConsistentInput(rawInputCircuit, secret_input, public_inputs, j, group);
	toReturn -> seed = seed;

	return toReturn;
}
