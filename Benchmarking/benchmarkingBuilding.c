void benchmarkLP_2010_CircuitBuilding(struct RawCircuit *rawInputCircuit)
{
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

	#pragma omp parallel for default(shared) private(i) schedule(auto)
	for(i = 0; i < numCircuits; i ++)
	{
		circuitsArray[i] = readInCircuit_FromRaw_Seeded_ConsistentInput(circuitCTXs[i], rawInputCircuit, consistentInputs[i], i, params);
	}
}