const int numBlocks = 1000;
const int blockSize = 100000;


void benchmarkRawCommSender(char *portNumStr)
{
	struct timespec timestamp_0, timestamp_1;
	clock_t c_0, c_1;

	struct sockaddr_in destWrite, destRead;
	int writeSocket, readSocket, mainWriteSock, mainReadSock;
	int writePort = atoi(portNumStr), readPort = writePort + 1;

	randctx *isaacCTX;
	unsigned char **inputs;
	int i, j;

	isaacCTX = (randctx*) calloc(1, sizeof(randctx));
	getIsaacContext(isaacCTX);

	inputs = (unsigned char **) calloc(numBlocks, sizeof(unsigned char *));
	for(i = 0; i < numBlocks; i ++)
	{
		inputs[i] = generateIsaacRandBytes(isaacCTX, blockSize, blockSize);
	}

	set_up_server_socket(destWrite, writeSocket, mainWriteSock, writePort);
	set_up_server_socket(destRead, readSocket, mainReadSock, readPort);

	c_0 = clock();
	timestamp_0 = timestamp();

	for(i = 0; i < numBlocks; i ++)
	{
		sendBoth(writeSocket, inputs[i], blockSize);
	}

	c_1 = clock();
	timestamp_1 = timestamp();

	printf("\nSending %d blocks of size %d\n", numBlocks, blockSize);
	printf("CPU time  :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
	printf("Wall time :     %lf\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));

	close_server_socket(writeSocket, mainWriteSock);
	close_server_socket(readSocket, mainReadSock);
}


void benchmarkRawCommReceiver(char *ipAddress, char *portNumStr)
{
	struct timespec timestamp_0, timestamp_1;
	clock_t c_0, c_1;

	struct sockaddr_in serv_addr_write, serv_addr_read;
	int writeSocket, readSocket;
	int readPort = atoi(portNumStr), writePort = readPort + 1;

	randctx *isaacCTX;
	unsigned char *commBuffer;
	int commBufferLen, i, j;

	isaacCTX = (randctx*) calloc(1, sizeof(randctx));
	getIsaacContext(isaacCTX);


	set_up_client_socket(readSocket, ipAddress, readPort, serv_addr_read);
	set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr_write);

	c_0 = clock();
	timestamp_0 = timestamp();

	for(i = 0; i < numBlocks; i ++)
	{
		commBuffer = receiveBoth(readSocket, commBufferLen);
		free(commBuffer);
	}

	c_1 = clock();
	timestamp_1 = timestamp();

	printf("\nReceiving %d blocks of size %d\n", numBlocks, blockSize);
	printf("CPU time  :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
	printf("Wall time :     %lf\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));

	close_client_socket(readSocket);
	close_client_socket(writeSocket);
}



void benchmarkECC()
{
	mpz_t plaintext_x, plaintext_y, SK;
	struct eccPoint *plaintext, *plaintextDot, *PK, **preComputes;
	struct ecc_Ciphertext *ciphertext;
	struct eccParams *params;
	gmp_randstate_t *state = seedRandGen();
	int i;

	struct timespec timestamp_0, timestamp_1;
	clock_t c_0, c_1;


	mpz_init(SK);
	mpz_init_set_ui(plaintext_x, 19);
	mpz_init_set_ui(plaintext_y, 72);

	params = initBrainpool_256_Curve();
	plaintext = initAndSetECC_Point(plaintext_x, plaintext_y, 0);
	plaintextDot = initAndSetECC_Point(plaintext_x, plaintext_y, 0);

	mpz_urandomm(SK, *state, params -> n);


	timestamp_0 = timestamp();
	c_0 = clock();
	preComputes = preComputePoints(params -> g, 512, params);
	for(i = 0; i < 2000; i ++)
	{
		PK = windowedScalarFixedPoint(SK, params -> g, preComputes, 9, params);
	}
	c_1 = clock();
	timestamp_1 = timestamp();
	printTiming(&timestamp_0, &timestamp_1, c_0, c_1, "Fixed Test 10");
	printPoint(PK);


	timestamp_0 = timestamp();
	c_0 = clock();
	for(i = 0; i < 2000; i ++)
	{
		PK = windowedScalarPoint(SK, params -> g, params);
	}
	c_1 = clock();
	timestamp_1 = timestamp();
	printTiming(&timestamp_0, &timestamp_1, c_0, c_1, "Not fixed Test");

	printPoint(PK);

}

