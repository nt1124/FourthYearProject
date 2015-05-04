void runBuilder_L_2013_HKE(struct RawCircuit *rawInputCircuit, struct idAndValue *startOfInputChain, char *portNumStr, randctx *ctx)
{
	struct sockaddr_in destWrite, destRead;
	int writeSocket, readSocket, mainWriteSock, mainReadSock;
	int writePort = atoi(portNumStr), readPort = writePort + 1;

	struct Circuit **circuitsArray;
	struct RawCircuit *rawCheckCircuit;
	int i, arrayLen, commBufferLen = 0, J_setSize = 0;
	const int lengthDelta = 40, check_stat_secParam = stat_SecParam + 6;

	struct timespec ext_t_0, ext_t_1;
	struct timespec int_t_0, int_t_1;
	clock_t ext_c_0, ext_c_1;
	clock_t int_c_0, int_c_1;

	gmp_randstate_t *state;
	struct eccParams *params;

	struct eccPoint **builderInputs, ***NP_consistentInputs, *C, *cTilde;
	struct builderInputCommitStruct *commitStruct;
	struct OT_NP_Receiver_Query **queries_Own;

	unsigned char *commBuffer, *J_set, ***bLists, ***hashedB_Lists, *delta;
	unsigned char **Xj_checkValues, ***OT_Inputs, *inputBitsOwn;

	ub4 **circuitSeeds = (ub4 **) calloc(stat_SecParam, sizeof(ub4*));
	randctx **circuitCTXs = (randctx **) calloc(stat_SecParam, sizeof(randctx*));
	mpz_t **aList;


	rawCheckCircuit = createRawCheckCircuit_No_OT_Opt_Keyed(rawInputCircuit -> numInputs_P1, lengthDelta);

	initRandGen();
	state = seedRandGen();
	Xj_checkValues = (unsigned char **) calloc(stat_SecParam, sizeof(unsigned char *));


	for(i = 0; i < stat_SecParam; i ++)
	{
		circuitCTXs[i] = (randctx*) calloc(1, sizeof(randctx));
		circuitSeeds[i] = getIsaacContext(circuitCTXs[i]);
	}


	set_up_server_socket(destWrite, writeSocket, mainWriteSock, writePort);
	set_up_server_socket(destRead, readSocket, mainReadSock, readPort);


	ext_t_0 = timestamp();
	ext_c_0 = clock();

	printf("Executor has connected to us.\n");


	int_t_0 = timestamp();
	int_c_0 = clock();

	params = initBrainpool_256_Curve();

	// Setup the Cs, then use these to create some consistent input for the main comp.
	C = setup_OT_NP_Sender(params, *state);
	cTilde = exchangeC_ForNaorPinkas(writeSocket, readSocket, C);
	aList = getNaorPinkasInputs(rawInputCircuit -> numInputs_P1, stat_SecParam, *state, params);
	NP_consistentInputs = computeNaorPinkasInputs(cTilde, aList, rawInputCircuit -> numInputs_P1, stat_SecParam, params);


	inputBitsOwn = convertChainIntoArray(startOfInputChain, rawInputCircuit -> numInputs_P1);

	delta = generateRandBytes(16, 16);

	bLists = generateConsistentOutputs(delta, rawInputCircuit -> numOutputs);
	hashedB_Lists = generateConsistentOutputsHashTables(bLists, rawInputCircuit -> numOutputs);

	for(i = 0; i < stat_SecParam; i ++)
	{
		Xj_checkValues[i] = generateRandBytes(16, 16);
	}

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Circuits Input Generation.");
	printAndZeroBothCounters();

	int_t_0 = timestamp();
	int_c_0 = clock();

	circuitsArray = buildAllCircuitsConsistentOutput(rawInputCircuit, startOfInputChain, NP_consistentInputs, bLists[0], bLists[1],
													params, circuitCTXs, stat_SecParam);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Circuits Building Done.");
	printAndZeroBothCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	OT_Inputs = getAllInputKeys(circuitsArray, stat_SecParam);
	full_CnC_OT_Mod_Sender_ECC(writeSocket, readSocket, rawInputCircuit -> numInputs_P2, OT_Inputs, Xj_checkValues, state, stat_SecParam, 1024);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "OT - Sender");
	printAndZeroBothCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	commBufferLen = 0;
	queries_Own = NaorPinkas_OT_Produce_Queries(rawInputCircuit -> numInputs_P1, inputBitsOwn, state, params, cTilde);
	commBuffer = serialiseQueries(queries_Own, rawInputCircuit -> numInputs_P1, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);

	for(i = 0; i < stat_SecParam; i++)
	{
		sendCircuit(writeSocket, readSocket, circuitsArray[i]);
	}

	// Builder sends the Hashed b Lists
	commBufferLen = 0;
	commBuffer = serialise3D_UChar_Array(hashedB_Lists, rawInputCircuit -> numOutputs, 16, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);


	int_c_1 = clock();
	int_t_1 = timestamp();

	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Circuits, queries and Hashed B's sent.");
	printAndZeroBothCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	commitStruct = makeCommitmentsBuilder(ctx, circuitsArray, state, stat_SecParam);
	commBuffer = serialiseC_Boxes(commitStruct, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Committed to inputs");
	printAndZeroBothCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	// Get the J-set proof from the Executor, then revealed all needed to build the J-set circuits, and the builder's
	// inputs for the non-J-set circuits.
	J_set = builder_decommitToJ_Set_L_2013_HKE(writeSocket, readSocket, circuitsArray, NP_consistentInputs,
											aList, inputBitsOwn, stat_SecParam, &J_setSize, circuitSeeds);
	
	commBuffer = serialiseBuilderInputBits_L_2013_HKE(circuitsArray, inputBitsOwn, rawInputCircuit -> numInputs_P1,
													J_set, stat_SecParam, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Decommit to J_set.");
	printAndZeroBothCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	// delta = (unsigned char *) calloc(16, sizeof(unsigned char));
	SC_DetectCheatingBuilder_HKE_Alt(writeSocket, readSocket, rawCheckCircuit,
									startOfInputChain, rawInputCircuit -> numInputs_P1, delta, lengthDelta,
									queries_Own, C, cTilde,
									check_stat_secParam, state, ctx);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Full Sub-Computation.");
	fflush(stdout);
	printAndZeroBothCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	commBufferLen = 0;
	commBuffer = serialise3D_UChar_Array(bLists, rawInputCircuit -> numOutputs, 16, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Decommit to B List");
	printAndZeroBothCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	commBuffer = Step5_CalculateLogarithms(NP_consistentInputs, aList, queries_Own, params, inputBitsOwn, J_set, stat_SecParam, rawInputCircuit -> numInputs_P1, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Main Logarithm Checks");
	printAndZeroBothCounters();


	ext_c_1 = clock();
	ext_t_1 = timestamp();

	freeRawCircuit(rawInputCircuit);

	printTiming(&ext_t_0, &ext_t_1, ext_c_0, ext_c_1, "Total time without connection setup");

	free_idAndValueChain(startOfInputChain);

	/*
	for(i = 0; i < stat_SecParam; i ++)
	{
		freeCircuitStruct(circuitsArray[i], 0);
	}
	*/

	close_server_socket(writeSocket, mainWriteSock);
	close_server_socket(readSocket, mainReadSock);

	printBothTotalCounters();
}






void runExecutor_L_2013_HKE(struct RawCircuit *rawInputCircuit, struct idAndValue *startOfInputChain, char *ipAddress, char *portNumStr, randctx *ctx)
{
	struct sockaddr_in serv_addr_write, serv_addr_read;
	int writeSocket, readSocket;
	int readPort = atoi(portNumStr), writePort = readPort + 1;
	const int lengthDelta = 40, check_stat_secParam = stat_SecParam + 6;
	int i, consistency;

	struct RawCircuit *rawCheckCircuit;
	struct Circuit **circuitsArray = (struct Circuit**) calloc(stat_SecParam, sizeof(struct Circuit*));

	struct jSetReveal_L_2013_HKE *secretsRevealed;
	struct publicInputsWithGroup *pubInputGroup;

	struct builderInputCommitStruct *partnersCommitStruct;
	struct eccPoint **builderInputs, ***NaorPinkasInputs, *C, *cTilde, **queries_Partner;
	struct eccParams *params;

	unsigned char *commBuffer, ***bLists, ***outputHashTable, **OT_Outputs;
	unsigned char *J_set, *deltaPrime, *permedInputs;
	unsigned char *cheatDetectOutput;

	gmp_randstate_t *state;

	int arrayLen, commBufferLen = 0, bufferOffset, J_setSize = 0, circuitsChecked = 0;

	struct timespec ext_t_0, ext_t_1;
	struct timespec int_t_0, int_t_1;
	clock_t ext_c_0, ext_c_1;
	clock_t int_c_0, int_c_1;



	set_up_client_socket(readSocket, ipAddress, readPort, serv_addr_read);
	set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr_write);
	ext_t_0 = timestamp();
	ext_c_0 = clock();

	params = initBrainpool_256_Curve();

	printf("Connected to builder.\n");

	state = seedRandGen();

	rawCheckCircuit = createRawCheckCircuit_No_OT_Opt_Keyed(rawInputCircuit -> numInputs_P1, lengthDelta);

	C = setup_OT_NP_Sender(params, *state);
	cTilde = exchangeC_ForNaorPinkas(writeSocket, readSocket, C);


	int_t_0 = timestamp();
	int_c_0 = clock();

	permedInputs = convertChainIntoArray(startOfInputChain, rawInputCircuit -> numInputs_P2);
	J_set = full_CnC_OT_Mod_Receiver_ECC_Alt(writeSocket, readSocket, &OT_Outputs, rawInputCircuit -> numInputs_P2,
											state, startOfInputChain, permedInputs, stat_SecParam, 1024);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "OT - Receiver");
	printAndZeroBothCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	queries_Partner = deserialiseQueries(commBuffer, rawInputCircuit -> numInputs_P1);

	for(i = 0; i < stat_SecParam; i ++)
	{
		circuitsArray[i] = receiveFullCircuit(writeSocket, readSocket);
	}

	// Executor receives the Hashed b Lists
	commBufferLen = 0;
	bufferOffset = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	outputHashTable = deserialise3D_UChar_Array(commBuffer, rawInputCircuit -> numOutputs, 16, &bufferOffset);
	free(commBuffer);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Receiving Circuits and auxiliary stuff.");
	printAndZeroBothCounters();


	int_c_0 = clock();
	int_t_0 = timestamp();

	commBufferLen = 0;
	bufferOffset = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	partnersCommitStruct = deserialiseC_Boxes(commBuffer, state, &bufferOffset);


	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Receiving commitments.");
	printAndZeroBothCounters();


	for(i = 0; i < stat_SecParam; i ++)
	{
		setCircuitsInputs_Values(startOfInputChain, circuitsArray[i], 0x00);
	}


	setInputsFromCharArray(circuitsArray, OT_Outputs, stat_SecParam);


	int_t_0 = timestamp();
	int_c_0 = clock();

	// Here we do the decommit...Getting the information we need for proving consistency later, and the
	// inputs for the builder's wires in the evaluation circuits.
	secretsRevealed = executor_ToJ_Set_L_2013_HKE(writeSocket, readSocket, circuitsArray, params, J_set, &J_setSize, stat_SecParam);
	commBuffer = receiveBoth(readSocket, commBufferLen);


	setBuilderInputs_L_2013_HKE(circuitsArray, secretsRevealed -> builderInputsEval, commBuffer,
								J_set, stat_SecParam, rawInputCircuit -> numInputs_P1);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Decommit to J_set.");
	printAndZeroBothCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	printf("\nEvaluating Circuits ");
	fflush(stdout);

	for(i = 0; i < stat_SecParam; i ++)
	{
		if(0x00 == J_set[i])
		{
			printf("%d, ", i);
			fflush(stdout);
			runCircuitExec( circuitsArray[i], writeSocket, readSocket );
		}
	}
	printf("\n");

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Evaluating all circuits");
	printAndZeroBothCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	deltaPrime = expandDeltaPrim(circuitsArray, J_set, stat_SecParam);
	cheatDetectOutput = SC_DetectCheatingExecutor_HKE_Alt(writeSocket, readSocket, rawCheckCircuit,
														rawInputCircuit -> numInputs_P1, deltaPrime, lengthDelta,
														queries_Partner, C, cTilde, check_stat_secParam, state, ctx);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Full Sub-Computation");
	printAndZeroBothCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	commBufferLen = 0;
	bufferOffset = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	bLists = deserialise3D_UChar_Array(commBuffer, rawInputCircuit -> numOutputs, 16, &bufferOffset);
	free(commBuffer);

	printf("\nVerified B_Lists = %d\n", verifyB_Lists(outputHashTable, bLists, circuitsArray[0] -> numInputsExecutor));

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Verifying B Lists");
	printAndZeroBothCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	circuitsChecked = L_2013_HKE_performCircuitChecks(circuitsArray, rawInputCircuit,
													C, secretsRevealed,
													bLists[0], bLists[1], params,
													J_set, J_setSize, stat_SecParam);


	printf("Circuits Correct = %d\n", circuitsChecked);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Checking Circuits Correct");
	printAndZeroBothCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();


	bufferOffset = 0;
	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	consistency = Step5_CheckLogarithms(commBuffer, secretsRevealed -> builderInputsEval, queries_Partner,
										params, J_set, stat_SecParam, rawInputCircuit -> numInputs_P1,
										&bufferOffset);

	printf("Consistency Check = %d\n", consistency);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Main Logarithms.");
	printAndZeroBothCounters();


	if(NULL == cheatDetectOutput)
	{
		printMajorityOutputAsBinary(circuitsArray, stat_SecParam, J_set);
	}
	else
	{
		setRawCircuitsInputs_Hardcode(startOfInputChain, rawInputCircuit -> gates);
		setRawCircuitsInputs_Hardcode(cheatDetectOutput, 0, rawInputCircuit -> numInputs_P1, rawInputCircuit -> gates);
		evaluateRawCircuit(rawInputCircuit);
		printOutputHexString_Raw(rawInputCircuit);
	}

	ext_c_1 = clock();
	ext_t_1 = timestamp();

	printTiming(&ext_t_0, &ext_t_1, ext_c_0, ext_c_1, "Total time without connection setup");


	close_client_socket(readSocket);
	close_client_socket(writeSocket);

	for(i = 0; i < stat_SecParam; i ++)
	{
		// freeCircuitStruct(circuitsArray[i], 1);
	}
	freeRawCircuit(rawInputCircuit);
	free_idAndValueChain(startOfInputChain);

	printBothTotalCounters();
}