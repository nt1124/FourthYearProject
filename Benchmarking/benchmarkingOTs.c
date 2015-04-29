void benchmark_OT_LP_CnC_Sender(char *portNumStr)
{
	struct timespec timestamp_0, timestamp_1;
	clock_t c_0, c_1;

	struct sockaddr_in destWrite, destRead;
	int writeSocket, readSocket, mainWriteSock, mainReadSock;
	int writePort = atoi(portNumStr), readPort = writePort + 1;

	gmp_randstate_t *state;

	randctx *isaacCTX;

	unsigned char ***OT_Inputs;
	const int numIterations = 10, numX = 128, numY = 10;
	int i, j, k;

	initRandGen();
	state = seedRandGen();


	isaacCTX = (randctx*) calloc(1, sizeof(randctx));
	getIsaacContext(isaacCTX);


	OT_Inputs = (unsigned char ***) calloc(2, sizeof(unsigned char **));
	OT_Inputs[0] = (unsigned char **) calloc(numX * numY, sizeof(unsigned char *));
	OT_Inputs[1] = (unsigned char **) calloc(numX * numY, sizeof(unsigned char *));
	for(i = 0; i < numX; i ++)
	{
		for(j = 0; j < numY; j ++)
		{
			k = i * numY + j;
			OT_Inputs[0][k] = generateIsaacRandBytes(isaacCTX, 16, 16);
			OT_Inputs[1][k] = generateIsaacRandBytes(isaacCTX, 16, 16);
		}
	}


	set_up_server_socket(destWrite, writeSocket, mainWriteSock, writePort);
	set_up_server_socket(destRead, readSocket, mainReadSock, readPort);

	c_0 = clock();
	timestamp_0 = timestamp();

	for(i = 0; i < numIterations; i ++)
	{
		printf(">>> %d\n", i);
		full_CnC_OT_Sender_ECC(writeSocket, readSocket, numX, OT_Inputs, state, numY, 1024);
	}

	c_1 = clock();
	timestamp_1 = timestamp();

	printf("\nLP_CnC OT %d times, %d by %d\n", numIterations, numX, numY);
	printf("CPU time  :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
	printf("Wall time :     %lf\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));

	close_server_socket(writeSocket, mainWriteSock);
	close_server_socket(readSocket, mainReadSock);
}



void benchmark_OT_LP_CnC_Receiver(char *ipAddress, char *portNumStr)
{
	struct timespec timestamp_0, timestamp_1;
	clock_t c_0, c_1;

	struct sockaddr_in serv_addr_write, serv_addr_read;
	int writeSocket, readSocket;
	int readPort = atoi(portNumStr), writePort = readPort + 1;

	gmp_randstate_t *state;
	randctx *isaacCTX;

	struct params_CnC_ECC *OT_params_R;
	struct otKeyPair_ECC **OT_keyPairs_R;

	unsigned char **OT_Outputs, *permedInputs;
	int i;
	const int numIterations = 10, numX = 128, numY = 10;



	initRandGen();
	state = seedRandGen();


	isaacCTX = (randctx*) calloc(1, sizeof(randctx));
	getIsaacContext(isaacCTX);
	permedInputs = (unsigned char *) calloc(numX, sizeof(unsigned char));

	set_up_client_socket(readSocket, ipAddress, readPort, serv_addr_read);
	set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr_write);

	c_0 = clock();
	timestamp_0 = timestamp();

	for(i = 0; i < numIterations; i ++)
	{
		printf(">>> %d\n", i);
	
		OT_params_R = OT_CnC_Receiver_Setup_Params(numX, state, permedInputs,
										numY, 1024);
		OT_keyPairs_R = OT_CnC_Receiver_Produce_Queries(OT_params_R, numX,
														state, permedInputs, numY);
		OT_CnC_Receiver_Send_Queries(writeSocket, readSocket, OT_params_R, OT_keyPairs_R, numX,
									state, permedInputs, numY);

		OT_CnC_Receiver_Transfer(writeSocket, readSocket, OT_params_R, OT_keyPairs_R,
								numX, state, permedInputs, numY);


		// full_CnC_OT_Receiver_ECC_Alt(writeSocket, readSocket, numX, state, permedInputs, &OT_Outputs, numY, 1024);
	}

	c_1 = clock();
	timestamp_1 = timestamp();

	printf("\nLP_CnC OT %d times, %d by %d\n", numIterations, numX, numY);
	printf("CPU time  :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
	printf("Wall time :     %lf\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));

	close_client_socket(readSocket);
	close_client_socket(writeSocket);
}




void benchmark_OT_L_CnC_Mod_Sender(char *portNumStr)
{
	struct timespec timestamp_0, timestamp_1;
	clock_t c_0, c_1;

	struct sockaddr_in destWrite, destRead;
	int writeSocket, readSocket, mainWriteSock, mainReadSock;
	int writePort = atoi(portNumStr), readPort = writePort + 1;

	gmp_randstate_t *state;

	randctx *isaacCTX;

	unsigned char ***OT_Inputs, **Xj_checkValues;
	const int numIterations = 10, numX = 128, numY = 10;
	int i, j, k;

	initRandGen();
	state = seedRandGen();


	isaacCTX = (randctx*) calloc(1, sizeof(randctx));
	getIsaacContext(isaacCTX);


	OT_Inputs = (unsigned char ***) calloc(2, sizeof(unsigned char **));
	OT_Inputs[0] = (unsigned char **) calloc(numX * numY, sizeof(unsigned char *));
	OT_Inputs[1] = (unsigned char **) calloc(numX * numY, sizeof(unsigned char *));
	for(i = 0; i < numX; i ++)
	{
		for(j = 0; j < numY; j ++)
		{
			k = i * numY + j;
			OT_Inputs[0][k] = generateIsaacRandBytes(isaacCTX, 16, 16);
			OT_Inputs[1][k] = generateIsaacRandBytes(isaacCTX, 16, 16);
		}
	}


	Xj_checkValues = (unsigned char **) calloc(numY, sizeof(unsigned char *));
	for(j = 0; j < numY; j ++)
	{
		Xj_checkValues[j] = generateIsaacRandBytes(isaacCTX, 16, 16);
	}

	set_up_server_socket(destWrite, writeSocket, mainWriteSock, writePort);
	set_up_server_socket(destRead, readSocket, mainReadSock, readPort);

	c_0 = clock();
	timestamp_0 = timestamp();

	for(i = 0; i < numIterations; i ++)
	{
		printf(">>> %d\n", i);
		full_CnC_OT_Mod_Sender_ECC(writeSocket, readSocket, numX, OT_Inputs, Xj_checkValues, state, numY, 1024);
	}

	c_1 = clock();
	timestamp_1 = timestamp();

	printf("\nLP_CnC OT %d times, %d by %d\n", numIterations, numX, numY);
	printf("CPU time  :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
	printf("Wall time :     %lf\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));

	close_server_socket(writeSocket, mainWriteSock);
	close_server_socket(readSocket, mainReadSock);
}



void benchmark_OT_L_CnC_Mod_Receiver(char *ipAddress, char *portNumStr)
{
	struct timespec timestamp_0, timestamp_1;
	clock_t c_0, c_1;

	struct sockaddr_in serv_addr_write, serv_addr_read;
	int writeSocket, readSocket;
	int readPort = atoi(portNumStr), writePort = readPort + 1;

	gmp_randstate_t *state;
	randctx *isaacCTX;
	struct idAndValue *startOfInputChain;

	unsigned char **OT_Outputs, *permedInputs;
	int i;
	const int numIterations = 10, numX = 128, numY = 10;



	initRandGen();
	state = seedRandGen();


	isaacCTX = (randctx*) calloc(1, sizeof(randctx));
	getIsaacContext(isaacCTX);
	permedInputs = (unsigned char *) calloc(numX, sizeof(unsigned char));

	startOfInputChain = convertArrayToChain(permedInputs, numX, 0);

	set_up_client_socket(readSocket, ipAddress, readPort, serv_addr_read);
	set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr_write);

	c_0 = clock();
	timestamp_0 = timestamp();

	for(i = 0; i < numIterations; i ++)
	{
		printf(">>> %d\n", i);
		full_CnC_OT_Mod_Receiver_ECC_Alt(writeSocket, readSocket, &OT_Outputs, numX, state, startOfInputChain, permedInputs, numY, 1024);
	}

	c_1 = clock();
	timestamp_1 = timestamp();

	printf("\nLP_CnC OT %d times, %d by %d\n", numIterations, numX, numY);
	printf("CPU time  :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
	printf("Wall time :     %lf\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));

	close_client_socket(readSocket);
	close_client_socket(writeSocket);
}



void benchmark_Symm_OT_NP_Sender(char *portNumStr)
{
	struct timespec timestamp_0, timestamp_1;
	clock_t c_0, c_1;

	struct sockaddr_in destWrite, destRead;
	int writeSocket, readSocket, mainWriteSock, mainReadSock;
	int writePort = atoi(portNumStr), readPort = writePort + 1;

	gmp_randstate_t *state;
	randctx *isaacCTX;

	struct OT_NP_Receiver_Query **queries_Own;
	struct eccPoint *C, *cTilde, **queries_Partner;
	struct eccParams *params;

	unsigned char ***OT_Inputs, **OT_Outputs, *permedInputs;
	int i, j, k;
	const int numIterations = 10, numX = 128, numY = 10;



	initRandGen();
	state = seedRandGen();
	params = initBrainpool_256_Curve();

	isaacCTX = (randctx*) calloc(1, sizeof(randctx));
	getIsaacContext(isaacCTX);
	permedInputs = (unsigned char *) calloc(numX, sizeof(unsigned char));


	OT_Inputs = (unsigned char ***) calloc(2, sizeof(unsigned char **));
	OT_Inputs[0] = (unsigned char **) calloc(numX * numY, sizeof(unsigned char *));
	OT_Inputs[1] = (unsigned char **) calloc(numX * numY, sizeof(unsigned char *));
	for(i = 0; i < numX; i ++)
	{
		for(j = 0; j < numY; j ++)
		{
			k = i * numY + j;
			OT_Inputs[0][k] = generateIsaacRandBytes(isaacCTX, 16, 16);
			OT_Inputs[1][k] = generateIsaacRandBytes(isaacCTX, 16, 16);
		}
	}

	set_up_server_socket(destWrite, writeSocket, mainWriteSock, writePort);
	set_up_server_socket(destRead, readSocket, mainReadSock, readPort);

	c_0 = clock();
	timestamp_0 = timestamp();

	for(i = 0; i < numIterations; i ++)
	{
		C = setup_OT_NP_Sender(params, *state);
		cTilde = exchangeC_ForNaorPinkas(writeSocket, readSocket, C);

		printf(">>> %d\n", i);
		// full_CnC_OT_Mod_Receiver_ECC_Alt(writeSocket, readSocket, &OT_Outputs, numX, state, startOfInputChain, permedInputs, numY, 1024);
		queries_Own = NaorPinkas_OT_Produce_Queries(numX, permedInputs, state, params, cTilde);
		queries_Partner = NaorPinkas_OT_Exchange_Queries(writeSocket, readSocket, numX,
														numX, queries_Own);

		NaorPinkas_OT_Sender_Transfer(writeSocket, readSocket, numX, OT_Inputs, state, numY, queries_Partner, params, C);
		OT_Outputs = NaorPinkas_OT_Receiver_Transfer(writeSocket, readSocket, numX, permedInputs, state, numY, queries_Own, params, cTilde);

	}

	c_1 = clock();
	timestamp_1 = timestamp();

	printf("\nOT_NP %d times, %d by %d\n", numIterations, numX, numY);
	printf("CPU time  :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
	printf("Wall time :     %lf\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));

	close_server_socket(writeSocket, mainWriteSock);
	close_server_socket(readSocket, mainReadSock);
}



void benchmark_Symm_OT_NP_Receiver(char *ipAddress, char *portNumStr)
{
	struct timespec timestamp_0, timestamp_1;
	clock_t c_0, c_1;

	struct sockaddr_in serv_addr_write, serv_addr_read;
	int writeSocket, readSocket;
	int readPort = atoi(portNumStr), writePort = readPort + 1;

	gmp_randstate_t *state;
	randctx *isaacCTX;

	struct OT_NP_Receiver_Query **queries_Own;
	struct eccPoint *C, *cTilde, **queries_Partner;
	struct eccParams *params;

	unsigned char ***OT_Inputs, **OT_Outputs, *permedInputs;
	int i, j, k;
	const int numIterations = 10, numX = 128, numY = 10;



	initRandGen();
	state = seedRandGen();
	params = initBrainpool_256_Curve();

	isaacCTX = (randctx*) calloc(1, sizeof(randctx));
	getIsaacContext(isaacCTX);
	permedInputs = (unsigned char *) calloc(numX, sizeof(unsigned char));


	OT_Inputs = (unsigned char ***) calloc(2, sizeof(unsigned char **));
	OT_Inputs[0] = (unsigned char **) calloc(numX * numY, sizeof(unsigned char *));
	OT_Inputs[1] = (unsigned char **) calloc(numX * numY, sizeof(unsigned char *));
	for(i = 0; i < numX; i ++)
	{
		for(j = 0; j < numY; j ++)
		{
			k = i * numY + j;
			OT_Inputs[0][k] = generateIsaacRandBytes(isaacCTX, 16, 16);
			OT_Inputs[1][k] = generateIsaacRandBytes(isaacCTX, 16, 16);
		}
	}

	set_up_client_socket(readSocket, ipAddress, readPort, serv_addr_read);
	set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr_write);

	c_0 = clock();
	timestamp_0 = timestamp();

	for(i = 0; i < numIterations; i ++)
	{
		C = setup_OT_NP_Sender(params, *state);
		cTilde = exchangeC_ForNaorPinkas(writeSocket, readSocket, C);

		printf(">>> %d\n", i);
		// full_CnC_OT_Mod_Receiver_ECC_Alt(writeSocket, readSocket, &OT_Outputs, numX, state, startOfInputChain, permedInputs, numY, 1024);
		queries_Own = NaorPinkas_OT_Produce_Queries(numX, permedInputs, state, params, cTilde);
		queries_Partner = NaorPinkas_OT_Exchange_Queries(writeSocket, readSocket, numX,
														numX, queries_Own);

		NaorPinkas_OT_Sender_Transfer(writeSocket, readSocket, numX, OT_Inputs, state, numY, queries_Partner, params, C);
		OT_Outputs = NaorPinkas_OT_Receiver_Transfer(writeSocket, readSocket, numX, permedInputs, state, numY, queries_Own, params, cTilde);

	}

	c_1 = clock();
	timestamp_1 = timestamp();

	printf("\nOT_NP %d times, %d by %d\n", numIterations, numX, numY);
	printf("CPU time  :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
	printf("Wall time :     %lf\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));

	close_client_socket(readSocket);
	close_client_socket(writeSocket);
}


