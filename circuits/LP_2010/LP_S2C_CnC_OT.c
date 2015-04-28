void runBuilder_LP_2010_CnC_OT(struct RawCircuit *rawInputCircuit, struct idAndValue *startOfInputChain,
							char *portNumStr, randctx *ctx)
{
	struct sockaddr_in destWrite, destRead;
	int writeSocket, readSocket, mainWriteSock, mainReadSock;
	int writePort = atoi(portNumStr), readPort = writePort + 1;
	int i, J_setSize = 0, numCircuits = stat_SecParam / 0.31;

	struct Circuit **circuitsArray;
	ub4 **circuitSeeds = (ub4 **) calloc(numCircuits, sizeof(ub4*));
	randctx **circuitCTXs = (randctx **) calloc(numCircuits, sizeof(randctx*));


	struct timespec ext_t_0, ext_t_1;
	struct timespec int_t_0, int_t_1;
	clock_t ext_c_0, ext_c_1;
	clock_t int_c_0, int_c_1;

	gmp_randstate_t *state;
	struct public_builderPRS_Keys *public_inputs;
	struct secret_builderPRS_Keys *secret_inputs;
	struct eccParams *params;

	struct eccPoint **builderInputs;
	int arrayLen;
	unsigned char *commBuffer, *J_set, ***OT_Inputs, tempByte;
	int commBufferLen = 0;


	initRandGen();
	state = seedRandGen();

	numCircuits += numCircuits % 2;


	for(i = 0; i < numCircuits; i ++)
	{
		circuitCTXs[i] = (randctx*) calloc(1, sizeof(randctx));
		circuitSeeds[i] = getIsaacContext(circuitCTXs[i]);
	}

	set_up_server_socket(destWrite, writeSocket, mainWriteSock, writePort);
	set_up_server_socket(destRead, readSocket, mainReadSock, readPort);


	ext_t_0 = timestamp();
	ext_c_0 = clock();

	params = initBrainpool_256_Curve();
	secret_inputs = generateSecrets(rawInputCircuit -> numInputs_P1, numCircuits, params, *state);
	public_inputs = computePublicInputs(secret_inputs, params);

	printf("Executor has connected to us.\n");

	int_t_0 = timestamp();
	int_c_0 = clock();

	circuitsArray = buildAllCircuits(rawInputCircuit, startOfInputChain, *state, numCircuits, params, secret_inputs, public_inputs, circuitCTXs, circuitSeeds);

	// Send all the public_builder_PRS_keys, thus commiting the Builder to the soon to follow circuits.
	sendPublicCommitments(writeSocket, readSocket, public_inputs, params);

	for(i = 0; i < numCircuits; i++)
	{
		sendCircuit(writeSocket, readSocket, circuitsArray[i]);
	}

	int_c_1 = clock();
	int_t_1 = timestamp();

	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "\nCircuit prep, building and sending.");
	printAndZeroBothCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	OT_Inputs = getAllInputKeys(circuitsArray, numCircuits);
	full_CnC_OT_Sender_ECC(writeSocket, readSocket, circuitsArray[0] -> numInputsExecutor, OT_Inputs, state, numCircuits, 1024);


	// At this point receive from the Executor the proof of the J-set.
	// Then provide the relevant r_j's.

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "OT - Sender");
	printAndZeroBothCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	J_set = builder_decommitToJ_Set(writeSocket, readSocket, circuitsArray, secret_inputs, numCircuits, &J_setSize, circuitSeeds);

	builderInputs =  computeBuilderInputs(public_inputs, secret_inputs,
								J_set, J_setSize, startOfInputChain, 
								params, &arrayLen);

	commBuffer = serialise_ECC_Point_Array(builderInputs, arrayLen, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Builder Cut and choose - Correctness etc.");
	printAndZeroBothCounters();

	int_t_0 = timestamp();
	int_c_0 = clock();

	proveConsistencyEvaluationKeys_Builder(writeSocket, readSocket, J_set, J_setSize, startOfInputChain,
											builderInputs, public_inputs -> public_keyPairs,
											public_inputs -> public_circuitKeys,
											public_inputs ->  numKeyPairs, public_inputs -> stat_SecParam, secret_inputs,
											params, state);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Proving consistency");
	printAndZeroBothCounters();

	ext_c_1 = clock();
	ext_t_1 = timestamp();

	freeRawCircuit(rawInputCircuit);

	printTiming(&ext_t_0, &ext_t_1, ext_c_0, ext_c_1, "\nTotal time without connection setup");

	free_idAndValueChain(startOfInputChain);

	for(i = 0; i < numCircuits; i ++)
	{
		freeCircuitStruct(circuitsArray[i], 0);
	}

	close_server_socket(writeSocket, mainWriteSock);
	close_server_socket(readSocket, mainReadSock);
}




void runExecutor_LP_2010_CnC_OT(struct RawCircuit *rawInputCircuit, struct idAndValue *startOfInputChain,
								char *ipAddress, char *portNumStr, randctx *ctx)
{
	struct sockaddr_in serv_addr_write, serv_addr_read;
	int writeSocket, readSocket;
	int readPort = atoi(portNumStr), writePort = readPort + 1;
	int i, commBufferLen = 0, arrayLen, J_setSize  = 0, circuitsChecked = 0, consistency;
	int numCircuits = stat_SecParam / 0.31;

	struct Circuit **circuitsArray = (struct Circuit**) calloc(numCircuits, sizeof(struct Circuit*));
	struct revealedCheckSecrets *secretsRevealed;
	struct publicInputsWithGroup *pubInputGroup;
	unsigned char *J_set, **output, *permedInputs;

	gmp_randstate_t *state;

	struct eccPoint **builderInputs;
	unsigned char *commBuffer;

	struct timespec ext_t_0, ext_t_1;
	struct timespec int_t_0, int_t_1;
	clock_t ext_c_0, ext_c_1;
	clock_t int_c_0, int_c_1;


	numCircuits += numCircuits % 2;


	set_up_client_socket(readSocket, ipAddress, readPort, serv_addr_read);
	set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr_write);

	ext_t_0 = timestamp();
	ext_c_0 = clock();

	printf("Connected to builder.\n");

	int_t_0 = timestamp();
	int_c_0 = clock();

	pubInputGroup = receivePublicCommitments(writeSocket, readSocket);

	for(i = 0; i < numCircuits; i ++)
	{
		circuitsArray[i] = receiveFullCircuit(writeSocket, readSocket);
	}

	for(i = 0; i < numCircuits; i ++)
	{
		setCircuitsInputs_Values(startOfInputChain, circuitsArray[i], 0x00);
	}
	// free_idAndValueChain(startOfInputChain);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "\nReceived Circuits etc.");
	printAndZeroBothCounters();



	int_t_0 = timestamp();
	int_c_0 = clock();

	state = seedRandGen();
	permedInputs = getPermedInputValuesExecutor(circuitsArray);

	J_set = full_CnC_OT_Receiver_ECC_Alt(writeSocket, readSocket, circuitsArray[0] -> numInputsExecutor, state, permedInputs, &output, numCircuits, 1024);

	setInputsFromCharArray(circuitsArray, output, numCircuits);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "OT - Receiver");

	int_t_0 = timestamp();
	int_c_0 = clock();

	// Here we do the decommit...
	secretsRevealed = executor_decommitToJ_Set(writeSocket, readSocket, circuitsArray, pubInputGroup -> public_inputs,
							pubInputGroup -> params, J_set, &J_setSize, numCircuits);


	circuitsChecked = secretInputsToCheckCircuits(circuitsArray, rawInputCircuit, pubInputGroup -> public_inputs,
												secretsRevealed -> revealedSecrets, secretsRevealed -> revealedCircuitSeeds,
												pubInputGroup -> params, J_set, J_setSize, numCircuits);

	printf("Circuits Check = %d\n\n", circuitsChecked);


	commBuffer = receiveBoth(readSocket, commBufferLen);
	commBufferLen = 0;
	builderInputs = deserialise_ECC_Point_Array(commBuffer, &arrayLen, &commBufferLen);
	free(commBuffer);

	setBuilderInputs(builderInputs, J_set, J_setSize, circuitsArray,
					pubInputGroup -> public_inputs, pubInputGroup -> params);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Executor Cut and choose - Correctness etc");
	printAndZeroBothCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	consistency = proveConsistencyEvaluationKeys_Exec(writeSocket, readSocket, J_set, J_setSize,
													builderInputs, pubInputGroup -> public_inputs -> public_keyPairs,
													pubInputGroup -> public_inputs -> public_circuitKeys,
													pubInputGroup -> public_inputs ->  numKeyPairs,
													pubInputGroup -> public_inputs -> stat_SecParam,
													pubInputGroup -> params, state);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Proving consistency");
	printAndZeroBothCounters();

	printf("Consistency Check = %d\n", consistency);

	int_c_0 = clock();
	int_t_0 = timestamp();

	printf("Evaluating Circuits ");
	for(i = 0; i < numCircuits; i ++)
	{
		if(0x00 == J_set[i])
		{
			printf("%d, ", i);
			fflush(stdout);
			runCircuitExec( circuitsArray[i], writeSocket, readSocket );
		}
	}
	freeRawCircuit(rawInputCircuit);
	printf("\n");

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Cirucit Evaluation");


	ext_c_1 = clock();
	ext_t_1 = timestamp();

	printTiming(&ext_t_0, &ext_t_1, ext_c_0, ext_c_1, "\nTotal time without connection setup");

	close_client_socket(readSocket);
	close_client_socket(writeSocket);

	// printMajorityOutputAsHex(circuitsArray, numCircuits, J_set);
	printMajorityOutputAsBinary(circuitsArray, numCircuits, J_set);

	// testAES_FromRandom();


	for(i = 0; i < numCircuits; i ++)
	{
		freeCircuitStruct(circuitsArray[i], 1);
	}
}