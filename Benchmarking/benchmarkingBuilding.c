void benchmark_LP_2010_CircuitBuilding(struct RawCircuit *rawInputCircuit)
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
}




void benchmark_L_2013_CircuitBuilding(struct RawCircuit *rawInputCircuit)
{
	struct timespec timestamp_0, timestamp_1;
	clock_t c_0, c_1;

	struct Circuit **circuitsArray;

	gmp_randstate_t *state;
	struct eccPoint ***consistentInputs;
	struct public_builderPRS_Keys *public_inputs;
	struct secret_builderPRS_Keys *secret_inputs;
	struct eccParams *params;

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
	printf("Wall time :     %lf\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));


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
}