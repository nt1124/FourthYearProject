struct Circuit **benchmark_LP_2010_CircuitBuilding(struct RawCircuit *rawInputCircuit)
{
	struct timespec timestamp_0, timestamp_1;
	clock_t c_0, c_1;

	struct Circuit **circuitsArray;

	gmp_randstate_t *state;
	struct eccPoint ***consistentInputs;
	struct public_builderPRS_Keys *public_inputs;
	struct secret_builderPRS_Keys *secret_inputs;
	struct eccParams *params;

	int numCircuits = 16;
	int i;

	unsigned char *inputBits;
	struct idAndValue *startOfInputChain;
	
	ub4 **circuitSeeds = (ub4 **) calloc(numCircuits, sizeof(ub4*));
	randctx **circuitCTXs = (randctx **) calloc(numCircuits, sizeof(randctx*));


	circuitsArray = (struct Circuit **) calloc(numCircuits, sizeof(struct Circuit*));

	c_0 = clock();
	timestamp_0 = timestamp();

	initRandGen();
	state = seedRandGen();

	params = initBrainpool_256_Curve();
	secret_inputs = generateSecrets(rawInputCircuit -> numInputs_P1, numCircuits, params, *state);
	public_inputs = computePublicInputs(secret_inputs, params);


	for(i = 0; i < numCircuits; i ++)
	{
		circuitCTXs[i] = (randctx*) calloc(1, sizeof(randctx));
		circuitSeeds[i] = getIsaacContext(circuitCTXs[i]);
	}

	consistentInputs = getAllConsistentInputsAsPoints(secret_inputs -> secret_circuitKeys, public_inputs -> public_keyPairs,
													params, numCircuits, rawInputCircuit -> numInputs_P1);

	c_1 = clock();
	timestamp_1 = timestamp();

	printf("\nLP_2010 input generation for %d circuits with %d Builder inputs\n", numCircuits, rawInputCircuit -> numInputs_P1);
	printf("CPU time  :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
	printf("Wall time :     %lf\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));


	c_0 = clock();
	timestamp_0 = timestamp();

	#pragma omp parallel for default(shared) private(i) schedule(auto)
	for(i = 0; i < numCircuits; i ++)
	{
		circuitsArray[i] = readInCircuit_FromRaw_Seeded_ConsistentInput(circuitCTXs[i], rawInputCircuit, consistentInputs[i], i, params);
	}

	c_1 = clock();
	timestamp_1 = timestamp();

	printf("\nLP_2010 Building %d circuits with %d Builder inputs\n", numCircuits, rawInputCircuit -> numInputs_P1);
	printf("CPU time  :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
	printf("Wall time :     %lf\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));


	inputBits = (unsigned char *) calloc(rawInputCircuit -> numInputs, sizeof(unsigned char));
	for(i = 0; i < rawInputCircuit -> numInputs; i ++)
	{
		inputBits[i] = rand() % 2;
	}
	startOfInputChain = convertArrayToChain(inputBits, rawInputCircuit -> numInputs, 0);

	c_0 = clock();
	timestamp_0= timestamp();

	for(i = 0; i < numCircuits; i ++)
	{
		setCircuitsInputs_Hardcode(startOfInputChain, circuitsArray[i], 0xFF);
		runCircuitExec(circuitsArray[i], 0, 0 );
	}

	c_1 = clock();
	timestamp_1 = timestamp();

	printf("\nLP_2010 Running %d circuits %d many gates\n", numCircuits, rawInputCircuit -> numGates);
	printf("CPU time  :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
	printf("Wall time :     %lf\n\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));

	return circuitsArray;
}




struct Circuit **benchmark_L_2013_CircuitBuilding(struct RawCircuit *rawInputCircuit)
{
	struct timespec timestamp_0, timestamp_1;
	clock_t c_0, c_1;

	struct Circuit **circuitsArray;

	gmp_randstate_t *state;
	struct eccPoint ***consistentInputs;
	struct public_builderPRS_Keys *public_inputs;
	struct secret_builderPRS_Keys *secret_inputs;
	struct eccParams *params;

	unsigned char *inputBits;
	struct idAndValue *startOfInputChain;

	unsigned char *delta, ***bLists, ***hashedB_Lists;

	int numCircuits = 16;
	int i;
	
	ub4 **circuitSeeds = (ub4 **) calloc(numCircuits, sizeof(ub4*));
	randctx **circuitCTXs = (randctx **) calloc(numCircuits, sizeof(randctx*));


	circuitsArray = (struct Circuit **) calloc(numCircuits, sizeof(struct Circuit*));

	c_0 = clock();
	timestamp_0 = timestamp();

	initRandGen();
	state = seedRandGen();

	params = initBrainpool_256_Curve();
	secret_inputs = generateSecrets(rawInputCircuit -> numInputs_P1, numCircuits, params, *state);
	public_inputs = computePublicInputs(secret_inputs, params);
	delta = generateRandBytes(16, 16);

	bLists = generateConsistentOutputs(delta, rawInputCircuit -> numOutputs);
	hashedB_Lists = generateConsistentOutputsHashTables(bLists, rawInputCircuit -> numOutputs);


	for(i = 0; i < numCircuits; i ++)
	{
		circuitCTXs[i] = (randctx*) calloc(1, sizeof(randctx));
		circuitSeeds[i] = getIsaacContext(circuitCTXs[i]);
	}

	consistentInputs = getAllConsistentInputsAsPoints(secret_inputs -> secret_circuitKeys, public_inputs -> public_keyPairs,
													params, numCircuits, rawInputCircuit -> numInputs_P1);

	c_1 = clock();
	timestamp_1 = timestamp();

	printf("\nL_2013 input generation for %d circuits with %d Builder inputs and %d Outputs\n", numCircuits, rawInputCircuit -> numInputs_P1, rawInputCircuit -> numOutputs);
	printf("CPU time  :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);


	inputBits = (unsigned char *) calloc(rawInputCircuit -> numInputs, sizeof(unsigned char));
	for(i = 0; i < rawInputCircuit -> numInputs; i ++)
	{
		inputBits[i] = rand() % 2;
	}
	startOfInputChain = convertArrayToChain(inputBits, rawInputCircuit -> numInputs, 0);


	c_0 = clock();
	timestamp_0 = timestamp();

	#pragma omp parallel for default(shared) private(i) schedule(auto)
	for(i = 0; i < numCircuits; i ++)
	{
		circuitsArray[i] = readInCircuit_FromRaw_Seeded_ConsistentInputOutput(circuitCTXs[i], rawInputCircuit, consistentInputs[i],
																			bLists[0], bLists[1], i, params);
	}

	c_1 = clock();
	timestamp_1 = timestamp();

	printf("\nL_2013 Building %d circuits with %d Builder inputs and %d Outputs\n", numCircuits, rawInputCircuit -> numInputs_P1, rawInputCircuit -> numOutputs);
	printf("CPU time  :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
	printf("Wall time :     %lf\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));


	inputBits = (unsigned char *) calloc(rawInputCircuit -> numInputs, sizeof(unsigned char));
	for(i = 0; i < rawInputCircuit -> numInputs; i ++)
	{
		inputBits[i] = rand() % 2;
	}
	startOfInputChain = convertArrayToChain(inputBits, rawInputCircuit -> numInputs, 0);

	c_0 = clock();
	timestamp_0= timestamp();

	for(i = 0; i < numCircuits; i ++)
	{
		setCircuitsInputs_Hardcode(startOfInputChain, circuitsArray[i], 0xFF);
		runCircuitExec(circuitsArray[i], 0, 0 );
	}

	c_1 = clock();
	timestamp_1 = timestamp();

	printf("\nL_2013 Running %d circuits %d many gates\n", numCircuits, rawInputCircuit -> numGates);
	printf("CPU time  :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
	printf("Wall time :     %lf\n\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));


	return circuitsArray;
}




struct Circuit **benchmark_HKE_2013_CircuitBuilding(struct RawCircuit *rawInputCircuit)
{
	struct timespec timestamp_0, timestamp_1;
	clock_t c_0, c_1;

	struct Circuit **circuitsArray;
	struct eccPoint ***NaorPinkasInputs, *cTilde;
	struct eccParams *params;
	mpz_t **aList, *outputKeysLocals;

	struct HKE_Output_Struct_Builder *outputStruct_Own;
	struct DDH_Group *groupOwn;

	unsigned char *inputBits;
	struct idAndValue *startOfInputChain;

	gmp_randstate_t *state;

	int numCircuits = 16, numOwnInputs, i, j;
	int partyID = 0;
	
	ub4 **circuitSeeds = (ub4 **) calloc(numCircuits, sizeof(ub4*));
	randctx **circuitCTXs = (randctx **) calloc(numCircuits, sizeof(randctx*));


	circuitsArray = (struct Circuit **) calloc(numCircuits, sizeof(struct Circuit*));

	c_0 = clock();
	timestamp_0 = timestamp();


	initRandGen();
	state = seedRandGen();
	params = initBrainpool_256_Curve();
	groupOwn = get_128_Bit_Group(*state);

	for(i = 0; i < numCircuits; i ++)
	{
		circuitCTXs[i] = (randctx*) calloc(1, sizeof(randctx));
		circuitSeeds[i] = getIsaacContext(circuitCTXs[i]);
	}

	outputStruct_Own = getOutputSecretsAndScheme(rawInputCircuit -> numOutputs, numCircuits, *state, groupOwn);
	// We cheat here and ignore getting a cTilde from a partner. Just assume it's passed in.
	cTilde = setup_OT_NP_Sender(params, *state);

	if(0 == partyID)
	{
		numOwnInputs = rawInputCircuit -> numInputs_P2;
	}
	else
	{
		numOwnInputs = rawInputCircuit -> numInputs_P1;
	}

	aList = getNaorPinkasInputs(numOwnInputs, numCircuits, *state, params);
	NaorPinkasInputs = computeNaorPinkasInputs(cTilde, aList, numOwnInputs, numCircuits, params);

	c_1 = clock();
	timestamp_1 = timestamp();
	printf("\nHKE_2013 input generation for %d circuits with %d Builder inputs and %d Outputs\n", numCircuits, rawInputCircuit -> numInputs_P1, rawInputCircuit -> numOutputs);
	printf("CPU time  :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
	printf("Wall time :     %lf\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));


	c_0 = clock();
	timestamp_0 = timestamp();

	#pragma omp parallel for default(shared) private(i, j, outputKeysLocals) schedule(auto)
	for(i = 0; i < numCircuits; i ++)
	{
		outputKeysLocals = getOutputKeys(outputStruct_Own, rawInputCircuit -> numOutputs, i);
		circuitsArray[i] = readInCircuit_FromRaw_HKE_2013(circuitCTXs[i], rawInputCircuit, NaorPinkasInputs[i], outputKeysLocals, params, partyID);

		for(j = 0; j < 2 * rawInputCircuit -> numOutputs; j ++)
		{
			mpz_clear(outputKeysLocals[j]);
		}
		free(outputKeysLocals);
	}

	c_1 = clock();
	timestamp_1 = timestamp();

	printf("\nHKE_2013 Building %d circuits with %d Builder inputs and %d Outputs\n", numCircuits, rawInputCircuit -> numInputs_P1, rawInputCircuit -> numOutputs);
	printf("CPU time  :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
	printf("Wall time :     %lf\n\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));

	inputBits = (unsigned char *) calloc(rawInputCircuit -> numInputs, sizeof(unsigned char));
	for(i = 0; i < rawInputCircuit -> numInputs; i ++)
	{
		inputBits[i] = rand() % 2;
	}
	startOfInputChain = convertArrayToChain(inputBits, rawInputCircuit -> numInputs, 0);

	c_0 = clock();
	timestamp_0= timestamp();

	for(i = 0; i < numCircuits; i ++)
	{
		setCircuitsInputs_Hardcode(startOfInputChain, circuitsArray[i], 0xFF);
		runCircuitExec(circuitsArray[i], 0, 0 );
	}

	c_1 = clock();
	timestamp_1 = timestamp();

	printf("\nHKE_2013 Running %d circuits %d many gates\n", numCircuits, rawInputCircuit -> numGates);
	printf("CPU time  :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
	printf("Wall time :     %lf\n\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));


	return circuitsArray;
}