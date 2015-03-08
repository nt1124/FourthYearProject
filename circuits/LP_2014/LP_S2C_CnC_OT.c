const int stat_SecParam = 8;

void runBuilder_LP_2014_CnC_OT(char *circuitFilepath, char *inputFilepath, char *portNumStr)
{
	struct sockaddr_in destWrite, destRead;
	int writeSocket, readSocket, mainWriteSock, mainReadSock;
	int writePort = atoi(portNumStr), readPort = writePort + 1;

	struct Circuit **circuitsArray;
	struct RawCircuit *rawInputCircuit = readInCircuit_Raw(circuitFilepath);
	struct idAndValue *startOfInputChain, *start;
	unsigned int *seedList;
	int i;


	struct timespec ext_t_0, ext_t_1;
	struct timespec int_t_0, int_t_1;
	clock_t ext_c_0, ext_c_1;
	clock_t int_c_0, int_c_1;

	gmp_randstate_t *state;
	struct public_builderPRS_Keys *public_inputs;
	struct secret_builderPRS_Keys *secret_inputs;
	struct eccParams *params;


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

	circuitsArray = buildAllCircuits(rawInputCircuit, inputFilepath, *state, stat_SecParam, seedList, params, secret_inputs, public_inputs);

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


	full_CnC_OT_Sender_ECC(writeSocket, readSocket, circuitsArray, state, stat_SecParam, 1024);

	// At this point receive from the Executor the proof of the J-set.
	// Then provide the relevant r_j's.
	builder_decommitToJ_Set(writeSocket, readSocket, circuitsArray, secret_inputs, stat_SecParam, seedList);


	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "OT - Sender");


	ext_c_1 = clock();
	ext_t_1 = timestamp();

	printTiming(&ext_t_0, &ext_t_1, ext_c_0, ext_c_1, "\nTotal time without connection setup");

	close_server_socket(writeSocket, mainWriteSock);
	close_server_socket(readSocket, mainReadSock);
}



void runExecutor_LP_2014_CnC_OT(char *circuitFilepath, char *inputFilepath, char *ipAddress, char *portNumStr)
{
	struct sockaddr_in serv_addr_write, serv_addr_read;
	int writeSocket, readSocket;
	int readPort = atoi(portNumStr), writePort = readPort + 1;
	int i;

	struct RawCircuit *rawInputCircuit;
	struct Circuit **circuitsArray = (struct Circuit**) calloc(stat_SecParam, sizeof(struct Circuit*));

	struct revealedCheckSecrets *secretsRevealed;
	struct publicInputsWithGroup *pubInputGroup;
	unsigned char *J_set;
	
	struct idAndValue *startOfInputChain;
	gmp_randstate_t *state;

	struct timespec ext_t_0, ext_t_1;
	clock_t ext_c_0, ext_c_1;


	set_up_client_socket(readSocket, ipAddress, readPort, serv_addr_read);
	set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr_write);

	ext_t_0 = timestamp();
	ext_c_0 = clock();

	printf("Connected to builder.\n");

	rawInputCircuit = readInCircuit_Raw(circuitFilepath);

	pubInputGroup = receivePublicCommitments(writeSocket, readSocket);

	for(i = 0; i < stat_SecParam; i ++)
	{
		circuitsArray[i] = receiveFullCircuit(writeSocket, readSocket);
	}

	startOfInputChain = readInputDetailsFile_Alt(inputFilepath);
	for(i = 0; i < stat_SecParam; i ++)
	{
		setCircuitsInputs_Values(startOfInputChain, circuitsArray[i], 0x00);
	}
	free_idAndValueChain(startOfInputChain);


	state = seedRandGen();
	J_set = full_CnC_OT_Receiver_ECC(writeSocket, readSocket, circuitsArray, state, stat_SecParam, 1024);


	// Here we do the decommit...
	secretsRevealed = executor_decommitToJ_Set(writeSocket, readSocket, circuitsArray, pubInputGroup -> public_inputs,
							pubInputGroup -> params, J_set, stat_SecParam);


	secretInputsToCheckCircuits(circuitsArray, rawInputCircuit,	pubInputGroup -> public_inputs,
								secretsRevealed -> revealedSecrets, secretsRevealed -> revealedSeeds, pubInputGroup -> params,
								J_set, stat_SecParam);


	for(i = 0; i < stat_SecParam; i ++)
	{
		if(0x00 == J_set[i])
		{
			runCircuitExec( circuitsArray[i], writeSocket, readSocket, inputFilepath );
		}
	}
	ext_c_1 = clock();
	ext_t_1 = timestamp();

	printTiming(&ext_t_0, &ext_t_1, ext_c_0, ext_c_1, "\nTotal time without connection setup");

	close_client_socket(readSocket);
	close_client_socket(writeSocket);

	printMajorityOutputAsHex(circuitsArray, stat_SecParam, J_set);

	testAES_FromRandom();

	for(i = 0; i < stat_SecParam; i ++)
	{
		freeCircuitStruct(circuitsArray[i]);
	}
}