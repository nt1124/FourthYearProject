struct Circuit **buildAll_HKE_Circuits(struct RawCircuit *rawInputCircuit, struct idAndValue *startOfInputChain,
									struct eccPoint *C, struct eccPoint ***NaorPinkasInputs,
									struct HKE_Output_Struct_Builder *outputStruct, struct eccParams *params,
									randctx *globalRandCTX, randctx **circuitCTXs, ub4 **circuitSeeds,
									int numCircuits, int partyID)
{
	struct Circuit **circuitsArray = (struct Circuit **) calloc(numCircuits, sizeof(struct Circuit*));
	struct idAndValue *start;
	int j;


	//#pragma omp parallel for private(j) schedule(auto)
	for(j = 0; j < numCircuits; j++)
	{
		circuitsArray[j] = readInCircuit_FromRaw_HKE_2013(circuitCTXs[j], rawInputCircuit, C, NaorPinkasInputs[j], outputStruct, j, params, partyID);
	}


	for(j = 0; j < numCircuits; j++)
	{
		start = startOfInputChain;
		setCircuitsInputs_Hardcode(start, circuitsArray[j], 0xFF);
	}


	return circuitsArray;
}




void runP1_HKE_2013_CnC_OT(struct RawCircuit *rawInputCircuit, struct idAndValue *startOfInputChain, char *portNumStr, randctx *ctx)
{
	struct sockaddr_in destWrite, destRead;
	int writeSocket, readSocket, mainWriteSock, mainReadSock;
	int writePort = atoi(portNumStr), readPort = writePort + 1;

	struct Circuit **circuitsArray;
	int i, commBufferLen = 0, J_setSize = 0;

	struct timespec ext_t_0, ext_t_1;
	struct timespec int_t_0, int_t_1;
	clock_t ext_c_0, ext_c_1;
	clock_t int_c_0, int_c_1;

	gmp_randstate_t *state;
	unsigned char *commBuffer, *J_set;
	struct eccParams *params;

	struct eccPoint ***NaorPinkasInputs, *C;
	struct builderInputCommitStruct *commitStruct, *tempP2CommitStruct;
	mpz_t **aList;

	struct HKE_Output_Struct_Builder *outputStruct;
	struct DDH_Group *group;
	int numCircuits = 8, tempLength = 0;

	ub4 **circuitSeeds = (ub4 **) calloc(numCircuits, sizeof(ub4*));
	randctx **circuitCTXs = (randctx **) calloc(numCircuits, sizeof(randctx*));


	initRandGen();
	state = seedRandGen();
	group = get_128_Bit_Group(*state);
	params = initBrainpool_256_Curve();


	for(i = 0; i < numCircuits; i ++)
	{
		circuitCTXs[i] = (randctx*) calloc(1, sizeof(randctx));
		circuitSeeds[i] = getIsaacContext(circuitCTXs[i]);
	}


	// set_up_server_socket(destWrite, writeSocket, mainWriteSock, writePort);
	// set_up_server_socket(destRead, readSocket, mainReadSock, readPort);


	struct idAndValue *startOfInputChainExec = readInputDetailsFile_Alt( (char*)"./inputs/adder_32bit.executor.input" );
	struct idAndValue *startOfInputChainBuilder = readInputDetailsFile_Alt( (char*)"./inputs/adder_32bit.builder.input" );


	ext_t_0 = timestamp();
	ext_c_0 = clock();

	printf("Executor has connected to us.\n");

	int_t_0 = timestamp();
	int_c_0 = clock();



	outputStruct = getOutputSecretsAndScheme(rawInputCircuit -> numOutputs, numCircuits, *state, group);
	C = setup_OT_NP_Sender(params, *state);
	aList = getNaorPinkasInputs(rawInputCircuit -> numInputs_P1, numCircuits, *state, params);
	NaorPinkasInputs = computeNaorPinkasInputs(C, aList, rawInputCircuit -> numInputs_P1, numCircuits, params);


	circuitsArray = buildAll_HKE_Circuits(rawInputCircuit, startOfInputChain, C, NaorPinkasInputs, outputStruct, params, ctx, circuitCTXs, circuitSeeds, numCircuits, 1);

	commitStruct = makeCommitmentsBuilder(ctx, circuitsArray, state, numCircuits);
	commBuffer = serialiseC_Boxes(commitStruct, &tempLength);
	
	commBufferLen = 0;
	tempP2CommitStruct = deserialiseC_Boxes(commBuffer, commitStruct -> params, commitStruct -> iMax, commitStruct -> jMax, state, &commBufferLen);

	ext_c_1 = clock();
	ext_t_1 = timestamp();


	printTiming(&ext_t_0, &ext_t_1, ext_c_0, ext_c_1, "\nTotal time without connection setup");


	for(i = 0; i < numCircuits; i ++)
	{
		setCircuitsInputs_Hardcode(startOfInputChainExec, circuitsArray[i], 0xFF);
		setCircuitsInputs_Hardcode(startOfInputChainBuilder, circuitsArray[i], 0xFF);
		runCircuitExec( circuitsArray[i], 0, 0 );
		printOutputHexString(circuitsArray[i]);
	}

	// close_server_socket(writeSocket, mainWriteSock);
	// close_server_socket(readSocket, mainReadSock);


	freeRawCircuit(rawInputCircuit);
	free_idAndValueChain(startOfInputChain);

	for(i = 0; i < numCircuits; i ++)
	{
		freeCircuitStruct(circuitsArray[i], 0);
	}
}




void runP2_HKE_2013_CnC_OT(struct RawCircuit *rawInputCircuit, struct idAndValue *startOfInputChain, char *portNumStr, randctx *ctx)
{
	struct sockaddr_in destWrite, destRead;
	int writeSocket, readSocket, mainWriteSock, mainReadSock;
	int writePort = atoi(portNumStr), readPort = writePort + 1;

	struct Circuit **circuitsArray;
	int i, commBufferLen = 0, J_setSize = 0;

	struct timespec ext_t_0, ext_t_1;
	struct timespec int_t_0, int_t_1;
	clock_t ext_c_0, ext_c_1;
	clock_t int_c_0, int_c_1;

	gmp_randstate_t *state;
	unsigned char *commBuffer, *J_set;
	struct eccParams *params;

	struct eccPoint ***NaorPinkasInputs, *C;
	struct builderInputCommitStruct *commitStruct, *tempP2CommitStruct;
	mpz_t **aList;

	struct HKE_Output_Struct_Builder *outputStruct;
	struct DDH_Group *group;
	int numCircuits = 8, tempLength = 0;

	ub4 **circuitSeeds = (ub4 **) calloc(numCircuits, sizeof(ub4*));
	randctx **circuitCTXs = (randctx **) calloc(numCircuits, sizeof(randctx*));


	initRandGen();
	state = seedRandGen();
	group = get_128_Bit_Group(*state);
	params = initBrainpool_256_Curve();


	for(i = 0; i < numCircuits; i ++)
	{
		circuitCTXs[i] = (randctx*) calloc(1, sizeof(randctx));
		circuitSeeds[i] = getIsaacContext(circuitCTXs[i]);
	}


	// set_up_server_socket(destWrite, writeSocket, mainWriteSock, writePort);
	// set_up_server_socket(destRead, readSocket, mainReadSock, readPort);


	struct idAndValue *startOfInputChainExec = readInputDetailsFile_Alt( (char*)"./inputs/adder_32bit.executor.input" );
	struct idAndValue *startOfInputChainBuilder = readInputDetailsFile_Alt( (char*)"./inputs/adder_32bit.builder.input" );


	ext_t_0 = timestamp();
	ext_c_0 = clock();

	printf("Executor has connected to us.\n");

	int_t_0 = timestamp();
	int_c_0 = clock();



	outputStruct = getOutputSecretsAndScheme(rawInputCircuit -> numOutputs, numCircuits, *state, group);
	C = setup_OT_NP_Sender(params, *state);
	aList = getNaorPinkasInputs(rawInputCircuit -> numInputs_P1, numCircuits, *state, params);
	NaorPinkasInputs = computeNaorPinkasInputs(C, aList, rawInputCircuit -> numInputs_P1, numCircuits, params);


	circuitsArray = buildAll_HKE_Circuits(rawInputCircuit, startOfInputChain, C, NaorPinkasInputs, outputStruct, params, ctx, circuitCTXs, circuitSeeds, numCircuits, 0);

	commitStruct = makeCommitmentsBuilder(ctx, circuitsArray, state, numCircuits);
	commBuffer = serialiseC_Boxes(commitStruct, &tempLength);
	
	commBufferLen = 0;
	tempP2CommitStruct = deserialiseC_Boxes(commBuffer, commitStruct -> params, commitStruct -> iMax, commitStruct -> jMax, state, &commBufferLen);

	ext_c_1 = clock();
	ext_t_1 = timestamp();


	printTiming(&ext_t_0, &ext_t_1, ext_c_0, ext_c_1, "\nTotal time without connection setup");


	for(i = 0; i < numCircuits; i ++)
	{
		setCircuitsInputs_Hardcode(startOfInputChainExec, circuitsArray[i], 0xFF);
		setCircuitsInputs_Hardcode(startOfInputChainBuilder, circuitsArray[i], 0xFF);
		runCircuitExec( circuitsArray[i], 0, 0 );
		printOutputHexString(circuitsArray[i]);
	}

	// close_server_socket(writeSocket, mainWriteSock);
	// close_server_socket(readSocket, mainReadSock);


	freeRawCircuit(rawInputCircuit);
	free_idAndValueChain(startOfInputChain);

	for(i = 0; i < numCircuits; i ++)
	{
		freeCircuitStruct(circuitsArray[i], 0);
	}
}
