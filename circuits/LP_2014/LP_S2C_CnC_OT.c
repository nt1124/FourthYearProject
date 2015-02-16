const int stat_SecParam = 1;

void runBuilder_LP_2014_CnC_OT(char *circuitFilepath, char *inputFilepath, char *portNumStr)
{
	struct sockaddr_in destWrite, destRead;
	int writeSocket, readSocket, mainWriteSock, mainReadSock;
	int writePort = atoi(portNumStr), readPort = writePort + 1;

	struct Circuit **circuitsArray;
	int i;

	struct idAndValue *startOfInputChain, *start;
	struct timespec ext_t_0, ext_t_1;
	struct timespec int_t_0, int_t_1;
	clock_t ext_c_0, ext_c_1;
	clock_t int_c_0, int_c_1;

	gmp_randstate_t *state;
	struct decParams *params;



	circuitsArray = (struct Circuit **) calloc(stat_SecParam, sizeof(struct Circuit*));


	set_up_server_socket(destWrite, writeSocket, mainWriteSock, writePort);
	set_up_server_socket(destRead, readSocket, mainReadSock, readPort);

	ext_t_0 = timestamp();
	ext_c_0 = clock();

	printf("Executor has connected to us.\n");

	int_t_0 = timestamp();
	int_c_0 = clock();

	for(i = 0; i < stat_SecParam; i++)
	{
		circuitsArray[i] = readInCircuitRTL(circuitFilepath);
	}

	startOfInputChain = readInputDetailsFile_Alt(inputFilepath);

	for(i = 0; i < stat_SecParam; i++)
	{
		start = startOfInputChain;
		setCircuitsInputs_Hardcode(start, circuitsArray[i], 0xFF);
	}
	free_idAndValueChain(startOfInputChain);

	int_c_1 = clock();
	int_t_1 = timestamp();

	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "\nBuilding/Inputting all Circuits");
	fflush(stdout);


	for(i = 0; i < stat_SecParam; i++)
	{
		sendCircuit(writeSocket, readSocket, circuitsArray[i]);
	}


	int_t_0 = timestamp();
	int_c_0 = clock();

	state = seedRandGen();

	/*
	params = senderCRS_Syn_Dec(writeSocket, readSocket, 1024, *state);
	for(i = 0; i < stat_SecParam; i++)
	{	
		builder_side_OT(writeSocket, readSocket, params, circuitsArray[i], state);

	}
	*/

	full_CnC_OT_Sender(writeSocket, readSocket, circuitsArray, state, stat_SecParam, 1024);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "OT - Sender");



	ext_c_1 = clock();
	ext_t_1 = timestamp();

	printTiming(&ext_t_0, &ext_t_1, ext_c_0, ext_c_1, "\nTotal time without connection setup");

	close_server_socket(writeSocket, mainWriteSock);
	close_server_socket(readSocket, mainReadSock);


	for(i = 0; i < stat_SecParam; i ++)
	{
		freeCircuitStruct(circuitsArray[i]);
	}
}



void runExecutor_LP_2014_CnC_OT(char *inputFilepath, char *ipAddress, char *portNumStr)
{
	struct sockaddr_in serv_addr_write, serv_addr_read;
	int writeSocket, readSocket;
	int readPort = atoi(portNumStr), writePort = readPort + 1;
	int i;

	int k;

	struct Circuit **circuitsArray = (struct Circuit**) calloc(stat_SecParam, sizeof(struct Circuit*));
	
	struct idAndValue *startOfInputChain;
	gmp_randstate_t *state;
	struct decParams *params;

	struct timespec ext_t_0, ext_t_1;
	clock_t ext_c_0, ext_c_1;


	set_up_client_socket(readSocket, ipAddress, readPort, serv_addr_read);
	set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr_write);

	ext_t_0 = timestamp();
	ext_c_0 = clock();

	printf("Connected to builder.\n");

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
	/*
	params = receiverCRS_Syn_Dec(writeSocket, readSocket);//, 1024, *state);
	for(i = 0; i < stat_SecParam; i ++)
	{
		executor_side_OT(writeSocket, readSocket, params, circuitsArray[i], state);
	}
	*/
	full_CnC_OT_Receiver(writeSocket, readSocket, circuitsArray, state, stat_SecParam, 1024);


	for(i = 0; i < stat_SecParam; i ++)
	{
		runCircuitExec( circuitsArray[i], writeSocket, readSocket, inputFilepath );
	}

	ext_c_1 = clock();
	ext_t_1 = timestamp();

	printTiming(&ext_t_0, &ext_t_1, ext_c_0, ext_c_1, "\nTotal time without connection setup");

	close_client_socket(readSocket);
	close_client_socket(writeSocket);

	printMajorityOutputAsHex(circuitsArray, stat_SecParam);
	// printOutputHexString(circuitsArray[0]);

	testAES_FromRandom();

	for(i = 0; i < stat_SecParam; i ++)
	{
		freeCircuitStruct(circuitsArray[i]);
	}
}