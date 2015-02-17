// RTL means the function deals with the Smart/Tillich style input.
struct gate *processGate_FromRaw(int numInputWires, int *inputIDs, int *rawOutputTable, char gateType)
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
struct gateOrWire *processGateOrWire_FromRaw(int idNum, int *inputIDs, int numInputWires,
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
	int numInputWires, purposelessNumber;
	int *inputIDs;

	numInputWires = rawGate -> numInputs;


	inputIDs = rawGate -> inputIDs;

	idNum = rawGate -> G_ID;

	return processGateOrWire_FromRaw(idNum, inputIDs, numInputWires, rawGate -> gateType, circuit, R, numInputs1);
}


// Initialises an input wire, needed because input wires don't feature in the input file.
// Therefore they need to initialised separately.
struct gateOrWire *initialiseInputWire_FromRaw(int idNum, unsigned char owner, unsigned char *R)
{
	struct gateOrWire *toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));

	toReturn -> G_ID = idNum;
	toReturn -> outputWire = (struct wire *) calloc(1, sizeof(struct wire));
	toReturn -> outputWire -> wirePerm = getPermutation();
	toReturn -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));

	// toReturn -> outputWire -> outputGarbleKeys = generateGarbleKeyPair(toReturn -> outputWire -> wirePerm);
	toReturn -> outputWire -> outputGarbleKeys = genFreeXORPairInput(toReturn -> outputWire -> wirePerm, R);

	toReturn -> gatePayload = NULL;
	toReturn -> outputWire -> wireMask = 0x01;
	toReturn -> outputWire -> wireOwner = owner;

	return toReturn;
}


// We assume party 1 is building the circuit.
struct gateOrWire **initialiseAllInputs_FromRaw(struct RawCircuit *rawInputCircuit, unsigned char *R)
{
	struct gateOrWire **gates = (struct gateOrWire**) calloc(rawInputCircuit -> numGates, sizeof(struct gateOrWire*));
	int i, numInputs = rawInputCircuit -> numInputsBuilder + rawInputCircuit -> numInputsExecutor;

	for(i = 0; i < rawInputCircuit -> numInputsBuilder; i ++)
	{
		gates[i] = initialiseInputWire(i, 0xFF, R);
	}

	for(; i < numInputs; i ++)
	{
		gates[i] = initialiseInputWire(i, 0x00, R);
	}


	return gates;
}


// Create a circuit given a file in RTL format.
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


	gatesList = initialiseAllInputs_FromRaw( rawInputCircuit, R );

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