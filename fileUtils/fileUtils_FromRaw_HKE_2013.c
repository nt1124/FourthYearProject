
// Initialises an input wire, needed because input wires don't feature in the input file.
// Therefore they need to initialised separately.
struct gateOrWire *initInputWire_FromRaw_HKE_2013(randctx *ctx, int idNum, unsigned char owner, unsigned char *R,
												struct eccPoint *NP_InputsZero, struct eccPoint *NP_InputsOne, struct eccParams *params)
{
	struct gateOrWire *toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));
	struct bitsGarbleKeys *tempOutput;

	toReturn -> G_ID = idNum;
	toReturn -> outputWire = (struct wire *) calloc(1, sizeof(struct wire));
	toReturn -> outputWire -> wirePerm = getIsaacPermutation(ctx);
	toReturn -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));

	tempOutput = (struct bitsGarbleKeys*) calloc(1, sizeof(struct bitsGarbleKeys));

	tempOutput -> key0 = hashECC_Point(NP_InputsZero, 16);
	tempOutput -> key1 = hashECC_Point(NP_InputsOne, 16);


	toReturn -> outputWire -> outputGarbleKeys = tempOutput;

	toReturn -> gatePayload = NULL;
	toReturn -> outputWire -> wireMask = 0x01;
	toReturn -> outputWire -> wireOwner = owner;

	return toReturn;
}


// partyID = 1 indicates the building party inputs come first.
// partyID = 0 indicates the building party inputs come second.
struct gateOrWire **initAllInputs_FromRaw_HKE_2013(randctx *ctx, struct RawCircuit *rawInputCircuit, unsigned char *R,
												struct eccPoint *C, struct eccPoint **NaorPinkasInputs, int partyID, struct eccParams *params)
{
	struct gateOrWire **gates = (struct gateOrWire**) calloc(rawInputCircuit -> numGates, sizeof(struct gateOrWire*));
	int i, index = 0;
	int execPartyStartID = 0, execPartyEndID, buildingPartyStartID = 0, buildingPartyEndID;


	if(1 == partyID)
	{
		execPartyStartID = rawInputCircuit -> numInputsBuilder;
	}
	else if(0 == partyID)
	{
		buildingPartyStartID = rawInputCircuit -> numInputsExecutor;
	}

	execPartyEndID = execPartyStartID + rawInputCircuit -> numInputsExecutor;
	buildingPartyEndID = buildingPartyStartID + rawInputCircuit -> numInputsBuilder;

	printf("%d => %d - %d  :  %d - %d\n", partyID, execPartyStartID, execPartyEndID, buildingPartyStartID, buildingPartyEndID);

	for(i = buildingPartyStartID; i < buildingPartyEndID; i ++)
	{
		gates[i] = initInputWire_FromRaw_HKE_2013(ctx, i, 0xFF, R, NaorPinkasInputs[index], NaorPinkasInputs[index + 1], params);
		index += 2;
	}


	for(i = execPartyStartID; i < execPartyEndID; i ++)
	{
		gates[i] = initInputWire_FromRaw(ctx, i, 0x00, R);
	}


	return gates;
}




// Create a circuit given a file in RTL format.
struct Circuit *readInCircuit_FromRaw_HKE_2013(randctx *ctx, struct RawCircuit *rawInputCircuit, struct eccPoint *C, struct eccPoint **NaorPinkasInputs, struct eccParams *params, int partyID)
{
	struct gateOrWire *tempGateOrWire;
	struct gateOrWire **gatesList;

	struct Circuit *outputCircuit = (struct Circuit*) calloc(1, sizeof(struct Circuit));
	int i, k = 0, gateIndex = 0;

	unsigned char *R = generateIsaacRandBytes(ctx, 16, 17);


	outputCircuit -> numGates = rawInputCircuit -> numGates;

	outputCircuit -> checkFlag = 0x00;
	outputCircuit -> numInputsBuilder = rawInputCircuit -> numInputsBuilder;
	outputCircuit -> numInputsExecutor = rawInputCircuit -> numInputsBuilder;
	outputCircuit -> numInputs = rawInputCircuit -> numInputsBuilder + rawInputCircuit -> numInputsExecutor;
	outputCircuit -> numOutputs = rawInputCircuit -> numOutputs;


	gatesList = initAllInputs_FromRaw_HKE_2013( ctx, rawInputCircuit, R, C, NaorPinkasInputs, partyID, params );

	for(i = outputCircuit -> numInputs; i < outputCircuit -> numGates; i ++)
	{
		gateIndex = rawInputCircuit -> execOrder[i];

		if(gateIndex < outputCircuit -> numGates - outputCircuit -> numOutputs)
		{
			tempGateOrWire = processGateLine_FromRaw(ctx, rawInputCircuit -> gates[gateIndex], gatesList, R, 0, outputCircuit -> numInputs);
		}
		else
		{
			tempGateOrWire = processGateOrWire_FromRaw(ctx, rawInputCircuit -> gates[gateIndex], gatesList, R, outputCircuit -> numInputs);
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


struct eccPoint ***computeNaorPinkasInputs(struct eccPoint *C, mpz_t **aLists, int numInputs, int numCircuits, struct eccParams *params)
{
	struct eccPoint ***output = (struct eccPoint ***) calloc(numCircuits, sizeof(struct eccPoint **));
	struct eccPoint **preComputes = preComputePoints(params -> g, 512, params), *invG_a1;
	int i, j, index;


	for(i = 0; i < numCircuits; i ++)
	{
		output[i] = (struct eccPoint **) calloc(2 * numInputs, sizeof(struct eccPoint *));
		index = 0;

		for(j = 0; j < numInputs; j ++)
		{
			output[i][index] = windowedScalarFixedPoint(aLists[i][index], params -> g, preComputes, 9, params);

			invG_a1 = windowedScalarFixedPoint(aLists[i][index + 1], params -> g, preComputes, 9, params);

			output[i][index + 1] = groupOp(C, invG_a1, params);
			clearECC_Point(invG_a1);

			index += 2;
		}
	}

	return output;
}


mpz_t **getNaorPinkasInputs(int numInputs, int numCircuits, gmp_randstate_t state, struct eccParams *params)
{
	mpz_t **output = (mpz_t **) calloc(numCircuits, sizeof(mpz_t *));
	mpz_t *temp;
	int i, j, index;


	for(i = 0; i < numCircuits; i ++)
	{
		output[i] = (mpz_t *) calloc(2 * numInputs, sizeof(mpz_t));
		index = 0;

		for(j = 0; j < numInputs; j ++)
		{
			mpz_init(output[i][index]);
			mpz_urandomm(output[i][index], state, params -> n);

			mpz_init(output[i][index + 1]);
			mpz_urandomm(output[i][index + 1], state, params -> n);

			index += 2;
		}
	}

	return output;
}


/*
void testHKE(char *circuitFilepath, char *ipAddress, char *portNumStr, char *inputFilename, int builder)
{
	struct timespec timestamp_0, timestamp_1;
	clock_t c_0, c_1;
	struct RawCircuit *rawInputCircuit = readInCircuit_Raw(circuitFilepath);
	struct idAndValue *startOfInputChain = readInputDetailsFile_Alt(inputFilename);
	struct eccParams *params = initBrainpool_256_Curve();

	struct eccPoint ***NaorPinkasInputs, *C;
	mpz_t **aList;
	gmp_randstate_t *state;
	int numCircuits = 1;


	struct idAndValue *startOfInputChainExec = readInputDetailsFile_Alt( (char*)"./inputs/adder_32bit.executor.input" );
	struct idAndValue *startOfInputChainBuilder = readInputDetailsFile_Alt( (char*)"./inputs/adder_32bit.builder.input" );

	state = seedRandGen();
	globalIsaacContext = (randctx*) calloc(1, sizeof(randctx));
	getIsaacContext(globalIsaacContext);

	c_0 = clock();
	timestamp_0 = timestamp();

	C = setup_OT_NP_Sender(params, *state);
	aList = getNaorPinkasInputs(rawInputCircuit -> numInputsBuilder, numCircuits, *state, params);
	NaorPinkasInputs = computeNaorPinkasInputs(C, aList, rawInputCircuit -> numInputsBuilder, numCircuits, params);

	struct Circuit *inputCircuit = readInCircuit_FromRaw_HKE_2013(globalIsaacContext, rawInputCircuit, C, NaorPinkasInputs[0], params, builder);


	printf(">>>>>>>>>\n");
	fflush(stdout);
	setCircuitsInputs_Hardcode(startOfInputChainExec, inputCircuit, 0xFF);
	setCircuitsInputs_Hardcode(startOfInputChainBuilder, inputCircuit, 0xFF);

	printf(">>>>>>>>>\n");
	fflush(stdout);

	runCircuitExec( inputCircuit, 0, 0 );
}
*/