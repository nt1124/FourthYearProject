// const int stat_SecParam = 4;


void runBuilder_LP_2010_CnC_OT(struct RawCircuit *rawInputCircuit, struct idAndValue *startOfInputChain, char *portNumStr)
{
	struct sockaddr_in destWrite, destRead;
	int writeSocket, readSocket, mainWriteSock, mainReadSock;
	int writePort = atoi(portNumStr), readPort = writePort + 1;

	struct Circuit **circuitsArray;
	unsigned int *seedList;
	int i, J_setSize = 0;


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
	unsigned char *commBuffer, *J_set, ***OT_Inputs;
	int commBufferLen = 0;


	initRandGen();
	state = seedRandGen();

	// group = getSchnorrGroup(1024, *state);
	params = initBrainpool_256_Curve();
	secret_inputs = generateSecrets(rawInputCircuit -> numInputsBuilder, stat_SecParam, params, *state);
	public_inputs = computePublicInputs(secret_inputs, params);


	set_up_server_socket(destWrite, writeSocket, mainWriteSock, writePort);
	set_up_server_socket(destRead, readSocket, mainReadSock, readPort);


	ext_t_0 = timestamp();
	ext_c_0 = clock();

	printf("Executor has connected to us.\n");

	int_t_0 = timestamp();
	int_c_0 = clock();

	seedList = generateRandUintList(stat_SecParam + 1);

	circuitsArray = buildAllCircuits(rawInputCircuit, startOfInputChain, *state, stat_SecParam, seedList, params, secret_inputs, public_inputs);

	srand(seedList[stat_SecParam]);

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


	int_t_0 = timestamp();
	int_c_0 = clock();


	OT_Inputs = getAllInputKeys(circuitsArray, stat_SecParam);
	full_CnC_OT_Sender_ECC(writeSocket, readSocket, circuitsArray[0] -> numInputsExecutor, OT_Inputs, state, stat_SecParam, 1024);

	// At this point receive from the Executor the proof of the J-set.
	// Then provide the relevant r_j's.
	J_set = builder_decommitToJ_Set(writeSocket, readSocket, circuitsArray, secret_inputs, stat_SecParam, &J_setSize, seedList);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "OT - Sender");


	builderInputs =  computeBuilderInputs(public_inputs, secret_inputs,
								J_set, J_setSize, startOfInputChain, 
								params, &arrayLen);

	commBuffer = serialise_ECC_Point_Array(builderInputs, arrayLen, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);


	proveConsistencyEvaluationKeys_Builder(writeSocket, readSocket, J_set, J_setSize, startOfInputChain,
											builderInputs, public_inputs, secret_inputs,
											params, state);

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



void runExecutor_LP_2010_CnC_OT(struct RawCircuit *rawInputCircuit, struct idAndValue *startOfInputChain, char *ipAddress, char *portNumStr)
{
	struct sockaddr_in serv_addr_write, serv_addr_read;
	int writeSocket, readSocket;
	int readPort = atoi(portNumStr), writePort = readPort + 1;
	int i, commBufferLen = 0, arrayLen, J_setSize  = 0 ,circuitsChecked = 0;

	// struct RawCircuit *rawInputCircuit;
	struct Circuit **circuitsArray = (struct Circuit**) calloc(stat_SecParam, sizeof(struct Circuit*));
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


	set_up_client_socket(readSocket, ipAddress, readPort, serv_addr_read);
	set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr_write);

	ext_t_0 = timestamp();
	ext_c_0 = clock();

	printf("Connected to builder.\n");

	int_t_0 = timestamp();
	int_c_0 = clock();

	fflush(stdout);
	pubInputGroup = receivePublicCommitments(writeSocket, readSocket);

	for(i = 0; i < stat_SecParam; i ++)
	{
		circuitsArray[i] = receiveFullCircuit(writeSocket, readSocket);
	}

	for(i = 0; i < stat_SecParam; i ++)
	{
		setCircuitsInputs_Values(startOfInputChain, circuitsArray[i], 0x00);
	}
	free_idAndValueChain(startOfInputChain);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "\nReceived Circuits");


	state = seedRandGen();
	permedInputs = getPermedInputValuesExecutor(circuitsArray);
	J_set = full_CnC_OT_Receiver_ECC_Alt(writeSocket, readSocket, circuitsArray[0] -> numInputsExecutor, state, permedInputs, &output, stat_SecParam, 1024);
	setInputsFromCharArray(circuitsArray, output, stat_SecParam);

	// Here we do the decommit...
	secretsRevealed = executor_decommitToJ_Set(writeSocket, readSocket, circuitsArray, pubInputGroup -> public_inputs,
							pubInputGroup -> params, J_set, &J_setSize, stat_SecParam);


	circuitsChecked = secretInputsToCheckCircuits(circuitsArray, rawInputCircuit, pubInputGroup -> public_inputs,
												secretsRevealed -> revealedSecrets, secretsRevealed -> revealedSeeds, pubInputGroup -> params,
												J_set, J_setSize, stat_SecParam);


	commBuffer = receiveBoth(readSocket, commBufferLen);
	commBufferLen = 0;
	builderInputs = deserialise_ECC_Point_Array(commBuffer, &arrayLen, &commBufferLen);
	free(commBuffer);

	setBuilderInputs(builderInputs, J_set, J_setSize, circuitsArray,
					pubInputGroup -> public_inputs, pubInputGroup -> params);


	proveConsistencyEvaluationKeys_Exec(writeSocket, readSocket, J_set, J_setSize,
										builderInputs, pubInputGroup -> public_inputs,
										pubInputGroup -> params, state);


	#pragma omp parallel for private(i) schedule(auto)
	for(i = 0; i < pubInputGroup -> public_inputs -> numKeyPairs; i ++)
	{
		clearECC_Point(pubInputGroup -> public_inputs -> public_keyPairs[i][0]);
		clearECC_Point(pubInputGroup -> public_inputs -> public_keyPairs[i][1]);

		free(pubInputGroup -> public_inputs -> public_keyPairs[i]);
	}
	#pragma omp parallel for private(i) schedule(auto)
	for(i = 0; i < pubInputGroup -> public_inputs -> stat_SecParam; i ++)
	{
		clearECC_Point(pubInputGroup -> public_inputs -> public_circuitKeys[i]);
	}
	free(pubInputGroup -> public_inputs -> public_circuitKeys);
	freeECC_Params(pubInputGroup -> params);


	#pragma omp parallel for private(i) schedule(auto)
	for(i = 0; i < arrayLen; i ++)
	{
		clearECC_Point(builderInputs[i]);
	}
	free(builderInputs);



	printf("Evaluating Circuits ");
	for(i = 0; i < stat_SecParam; i ++)
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

	ext_c_1 = clock();
	ext_t_1 = timestamp();

	printTiming(&ext_t_0, &ext_t_1, ext_c_0, ext_c_1, "\nTotal time without connection setup");

	close_client_socket(readSocket);
	close_client_socket(writeSocket);

	printMajorityOutputAsHex(circuitsArray, stat_SecParam, J_set);

	testAES_FromRandom();


	for(i = 0; i < stat_SecParam; i ++)
	{
		freeCircuitStruct(circuitsArray[i], 1);
	}
}