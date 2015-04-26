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
	int i;

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





void benchmark_ECC_PointSender(char *portNumStr)
{
	struct timespec timestamp_0, timestamp_1;
	clock_t c_0, c_1;

	struct sockaddr_in destWrite, destRead;
	int writeSocket, readSocket, mainWriteSock, mainReadSock;
	int writePort = atoi(portNumStr), readPort = writePort + 1;

	gmp_randstate_t *state;
	struct eccParams *params;

	unsigned char *commBuffer;
	struct eccPoint **inputPoints, **preComputes;
	mpz_t tempExp;
	int i, commBufferLen;


	mpz_init(tempExp);
	initRandGen();
	state = seedRandGen();
	params = initBrainpool_256_Curve();
	struct eccPoint **localPreComputes = fixedBasePreComputes(params -> g, params);




	inputPoints = (struct eccPoint **) calloc(numBlocks, sizeof(struct eccPoint *));
	for(i = 0; i < numBlocks; i ++)
	{
		mpz_urandomm(tempExp, *state, params -> n);
		inputPoints[i] = fixedPointMultiplication(localPreComputes, tempExp, params);
	}


	set_up_server_socket(destWrite, writeSocket, mainWriteSock, writePort);
	set_up_server_socket(destRead, readSocket, mainReadSock, readPort);

	c_0 = clock();
	timestamp_0 = timestamp();

	for(i = 0; i < numBlocks; i ++)
	{
		commBufferLen = 0;
		commBuffer = serialise_ECC_Point_Array(inputPoints, numBlocks, &commBufferLen);
		sendBoth(writeSocket, commBuffer, commBufferLen);
		free(commBuffer);
	}

	c_1 = clock();
	timestamp_1 = timestamp();

	printf("\nSending %d blocks of %d points\n", numBlocks, numBlocks);
	printf("CPU time  :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
	printf("Wall time :     %lf\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));

	close_server_socket(writeSocket, mainWriteSock);
	close_server_socket(readSocket, mainReadSock);
}



void benchmark_ECC_PointReceiver(char *ipAddress, char *portNumStr)
{
	struct timespec timestamp_0, timestamp_1;
	clock_t c_0, c_1;

	struct sockaddr_in serv_addr_write, serv_addr_read;
	int writeSocket, readSocket;
	int readPort = atoi(portNumStr), writePort = readPort + 1;

	struct eccPoint **outputPoints;
	unsigned char *commBuffer;
	int commBufferLen, bufferOffset = 0, arrayLen = 0, i;


	set_up_client_socket(readSocket, ipAddress, readPort, serv_addr_read);
	set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr_write);

	c_0 = clock();
	timestamp_0 = timestamp();

	for(i = 0; i < numBlocks; i ++)
	{
		bufferOffset = 0;
		arrayLen = 0;
		commBuffer = receiveBoth(readSocket, commBufferLen);
		outputPoints = deserialise_ECC_Point_Array(commBuffer, &arrayLen, &bufferOffset);
		free(commBuffer);
	}

	c_1 = clock();
	timestamp_1 = timestamp();

	printf("\nReceiving %d blocks of %d points\n", numBlocks, numBlocks);
	printf("CPU time  :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
	printf("Wall time :     %lf\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));

	close_client_socket(readSocket);
	close_client_socket(writeSocket);
}
