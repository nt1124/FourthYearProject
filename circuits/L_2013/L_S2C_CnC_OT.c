void runBuilder_L_2013_CnC_OT(struct RawCircuit *rawInputCircuit, struct idAndValue *startOfInputChain, char *portNumStr, randctx *ctx)
{
	struct sockaddr_in destWrite, destRead;
	int writeSocket, readSocket, mainWriteSock, mainReadSock;
	int writePort = atoi(portNumStr), readPort = writePort + 1;

	struct Circuit **circuitsArray;
	struct RawCircuit *rawCheckCircuit = createRawCheckCircuit(rawInputCircuit -> numInputs_P1);
	int i, arrayLen, commBufferLen = 0, J_setSize = 0;
	const int lengthDelta = 40;

	struct timespec ext_t_0, ext_t_1;
	struct timespec int_t_0, int_t_1;
	clock_t ext_c_0, ext_c_1;
	clock_t int_c_0, int_c_1;

	gmp_randstate_t *state;
	struct public_builderPRS_Keys *public_inputs;
	struct secret_builderPRS_Keys *secret_inputs, *checkSecretInputs;
	struct eccParams *params;
	struct secCompBuilderOutput *SC_ReturnStruct;

	struct eccPoint **builderInputs, ***consistentInputs;
	unsigned char *commBuffer, *J_set, ***bLists, ***hashedB_Lists, *delta;
	unsigned char **Xj_checkValues, ***OT_Inputs;

	ub4 **circuitSeeds = (ub4 **) calloc(stat_SecParam, sizeof(ub4*));
	randctx **circuitCTXs = (randctx **) calloc(stat_SecParam, sizeof(randctx*));


	initRandGen();
	state = seedRandGen();
	Xj_checkValues = (unsigned char **) calloc(stat_SecParam, sizeof(unsigned char *));


	for(i = 0; i < stat_SecParam; i ++)
	{
		circuitCTXs[i] = (randctx*) calloc(1, sizeof(randctx));
		circuitSeeds[i] = getIsaacContext(circuitCTXs[i]);
	}

	params = initBrainpool_256_Curve();
	secret_inputs = generateSecrets(rawInputCircuit -> numInputs_P1, stat_SecParam, params, *state);
	public_inputs = computePublicInputs(secret_inputs, params);
	delta = generateRandBytes(16, 16);

	bLists = generateConsistentOutputs(delta, rawInputCircuit -> numOutputs);
	hashedB_Lists = generateConsistentOutputsHashTables(bLists, rawInputCircuit -> numOutputs);

	set_up_server_socket(destWrite, writeSocket, mainWriteSock, writePort);
	set_up_server_socket(destRead, readSocket, mainReadSock, readPort);


	ext_t_0 = timestamp();
	ext_c_0 = clock();

	printf("Executor has connected to us.\n");

	int_t_0 = timestamp();
	int_c_0 = clock();


	for(i = 0; i < stat_SecParam; i ++)
	{
		Xj_checkValues[i] = generateRandBytes(16, 16);		
	}

	circuitsArray = buildAllCircuitsConsistentOutput(rawInputCircuit, startOfInputChain, *state, stat_SecParam, bLists[0], bLists[1],
													params, secret_inputs, public_inputs, circuitCTXs, circuitSeeds);

	int_c_1 = clock();
	int_t_1 = timestamp();

	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "\nBuilding/Inputting all Circuits");
	fflush(stdout);


	// Send all the public_builder_PRS_keys, thus commiting the Builder to the soon to follow circuits.
	sendPublicCommitments(writeSocket, readSocket, public_inputs, params);

	for(i = 0; i < stat_SecParam; i++)
	{
		sendCircuit(writeSocket, readSocket, circuitsArray[i]);
	}

	commBufferLen = 0;
	commBuffer = serialise3D_UChar_Array(hashedB_Lists, rawInputCircuit -> numOutputs, 16, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);


	int_t_0 = timestamp();
	int_c_0 = clock();

	OT_Inputs = getAllInputKeys(circuitsArray, stat_SecParam);
	full_CnC_OT_Mod_Sender_ECC(writeSocket, readSocket, circuitsArray[0] -> numInputsExecutor, OT_Inputs, Xj_checkValues, state, stat_SecParam, 1024);

	// At this point receive from the Executor the proof of the J-set. Then provide the relevant r_j's.
	J_set = builder_decommitToJ_Set(writeSocket, readSocket, circuitsArray, secret_inputs, stat_SecParam, &J_setSize, circuitSeeds);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "OT - Sender");


	builderInputs = computeBuilderInputs(public_inputs, secret_inputs,
								J_set, J_setSize, startOfInputChain, 
								params, &arrayLen);

	commBuffer = serialise_ECC_Point_Array(builderInputs, arrayLen, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);


	checkSecretInputs = generateSecretsCheckComp(rawInputCircuit -> numInputs_P1, 3 * stat_SecParam,
												secret_inputs, params, *state);


	SC_ReturnStruct = SC_DetectCheatingBuilder(writeSocket, readSocket, rawCheckCircuit,
											startOfInputChain, delta, lengthDelta,
	 										checkSecretInputs, 3 * stat_SecParam, state);

	commBufferLen = 0;
	commBuffer = serialise3D_UChar_Array(bLists, rawInputCircuit -> numOutputs, 16, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);


	int_t_0 = timestamp();
	int_c_0 = clock();

	proveConsistencyEvaluationKeys_Builder_L_2013(writeSocket, readSocket, J_set, J_setSize,
											startOfInputChain, builderInputs,
											public_inputs, secret_inputs, params, SC_ReturnStruct, state);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Proving consistency");

	ext_c_1 = clock();
	ext_t_1 = timestamp();

	freeRawCircuit(rawInputCircuit);

	printTiming(&ext_t_0, &ext_t_1, ext_c_0, ext_c_1, "\nTotal time without connection setup");

	free_idAndValueChain(startOfInputChain);

	for(i = 0; i < stat_SecParam; i ++)
	{
		freeCircuitStruct(circuitsArray[i], 0);
	}

	close_server_socket(writeSocket, mainWriteSock);
	close_server_socket(readSocket, mainReadSock);
}




void runExecutor_L_2013_CnC_OT(struct RawCircuit *rawInputCircuit, struct idAndValue *startOfInputChain, char *ipAddress, char *portNumStr, randctx *ctx)
{
	struct sockaddr_in serv_addr_write, serv_addr_read;
	int writeSocket, readSocket;
	int readPort = atoi(portNumStr), writePort = readPort + 1;
	const int lengthDelta = 40;
	int i, consistency;

	struct RawCircuit *rawCheckCircuit;
	struct Circuit **circuitsArray = (struct Circuit**) calloc(stat_SecParam, sizeof(struct Circuit*));

	struct revealedCheckSecrets *secretsRevealed;
	struct publicInputsWithGroup *pubInputGroup;
	struct secCompExecutorOutput *SC_ReturnStruct;
	unsigned char *J_set, *deltaPrime, *permedInputs, **OT_Outputs;

	gmp_randstate_t *state;

	struct eccPoint **builderInputs, ***consistentInputs;
	int arrayLen, commBufferLen = 0, bufferOffset, J_setSize = 0, circuitsChecked = 0;
	unsigned char *commBuffer, ***bLists, ***outputHashTable;

	struct timespec ext_t_0, ext_t_1;
	struct timespec int_t_0, int_t_1;
	clock_t ext_c_0, ext_c_1;
	clock_t int_c_0, int_c_1;


	set_up_client_socket(readSocket, ipAddress, readPort, serv_addr_read);
	set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr_write);

	ext_t_0 = timestamp();
	ext_c_0 = clock();

	printf("Connected to builder.\n");

	rawCheckCircuit = createRawCheckCircuit(rawInputCircuit -> numInputs_P1);

	int_t_0 = timestamp();
	int_c_0 = clock();
	pubInputGroup = receivePublicCommitments(writeSocket, readSocket);

	for(i = 0; i < stat_SecParam; i ++)
	{
		circuitsArray[i] = receiveFullCircuit(writeSocket, readSocket);
	}

	commBufferLen = 0;
	bufferOffset = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	outputHashTable = deserialise3D_UChar_Array(commBuffer, rawInputCircuit -> numOutputs, 16, &bufferOffset);
	free(commBuffer);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Receiving Circuits");


	for(i = 0; i < stat_SecParam; i ++)
	{
		setCircuitsInputs_Values(startOfInputChain, circuitsArray[i], 0x00);
	}

	int_t_0 = timestamp();
	int_c_0 = clock();

	state = seedRandGen();
	permedInputs = getPermedInputValuesExecutor(circuitsArray);
	// J_set = full_CnC_OT_Mod_Receiver_ECC(writeSocket, readSocket, circuitsArray, state, startOfInputChain, permedInputs, stat_SecParam, 1024);
	J_set = full_CnC_OT_Mod_Receiver_ECC_Alt(writeSocket, readSocket, &OT_Outputs, circuitsArray[0] -> numInputsExecutor,
											state, startOfInputChain, permedInputs, stat_SecParam, 1024);
	setInputsFromCharArray(circuitsArray, OT_Outputs, stat_SecParam);

	// Here we do the decommit...
	secretsRevealed = executor_decommitToJ_Set(writeSocket, readSocket, circuitsArray, pubInputGroup -> public_inputs,
							pubInputGroup -> params, J_set, &J_setSize, stat_SecParam);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "OT - Receiver");


	commBufferLen = 0;
	bufferOffset = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	builderInputs = deserialise_ECC_Point_Array(commBuffer, &arrayLen, &bufferOffset);
	free(commBuffer);


	setBuilderInputs(builderInputs, J_set, J_setSize, circuitsArray,
					pubInputGroup -> public_inputs, pubInputGroup -> params);

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

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "\n\nEvaluating all circuits");


	deltaPrime = expandDeltaPrim(circuitsArray, J_set, stat_SecParam);
	SC_ReturnStruct = SC_DetectCheatingExecutor(writeSocket, readSocket, rawCheckCircuit,
	 											deltaPrime, lengthDelta, 3 * stat_SecParam, state );


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
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "\nVerifying B Lists");


	int_t_0 = timestamp();
	int_c_0 = clock();


	consistentInputs = getAllConsistentInputsAsPoints(secretsRevealed -> revealedSecrets, pubInputGroup -> public_inputs -> public_keyPairs, pubInputGroup -> params, J_set, stat_SecParam, rawInputCircuit -> numInputs_P1);
	circuitsChecked = secretInputsToCheckCircuitsConsistentOutputs(circuitsArray, rawInputCircuit, consistentInputs, 
																secretsRevealed -> revealedCircuitSeeds, bLists[0], bLists[1], pubInputGroup -> params,
																J_set, J_setSize, stat_SecParam);

	printf("Circuits Correct = %d\n", circuitsChecked);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "\nChecking Circuits Correct");


	int_t_0 = timestamp();
	int_c_0 = clock();

	proveConsistencyEvaluationKeys_Exec_L_2013(writeSocket, readSocket, J_set, J_setSize,
											builderInputs, pubInputGroup -> public_inputs,
											pubInputGroup -> params,
											SC_ReturnStruct, state);
	
	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "\nBuilder Inputs consistency checked");

	clearPublicInputsWithGroup(pubInputGroup);

	#pragma omp parallel for private(i) schedule(auto)
	for(i = 0; i < arrayLen; i ++)
	{
		clearECC_Point(builderInputs[i]);
	}
	free(builderInputs);


	ext_c_1 = clock();
	ext_t_1 = timestamp();

	printTiming(&ext_t_0, &ext_t_1, ext_c_0, ext_c_1, "\nTotal time without connection setup");

	close_client_socket(readSocket);
	close_client_socket(writeSocket);

	if(1 || NULL == SC_ReturnStruct -> output)
	{
		printMajorityOutputAsBinary(circuitsArray, stat_SecParam, J_set);
	}
	else
	{
		setRawCircuitsInputs_Hardcode(startOfInputChain, rawInputCircuit -> gates);
		setRawCircuitsInputs_Hardcode(SC_ReturnStruct -> output, 0, rawInputCircuit -> numInputs_P1, rawInputCircuit -> gates);
		evaluateRawCircuit(rawInputCircuit);
		printOutputHexString_Raw(rawInputCircuit);
	}

	// testAES_FromRandom();

	for(i = 0; i < stat_SecParam; i ++)
	{
		freeCircuitStruct(circuitsArray[i], 1);
	}
	freeRawCircuit(rawInputCircuit);
	free_idAndValueChain(startOfInputChain);
}