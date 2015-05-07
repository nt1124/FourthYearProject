

void benchmarkRawCommSender(char *portNumStr)
{
	struct timespec timestamp_0, timestamp_1;
	clock_t c_0, c_1;

	struct sockaddr_in destWrite, destRead;
	int writeSocket, readSocket, mainWriteSock, mainReadSock;
	int writePort = atoi(portNumStr), readPort = writePort + 1;

	randctx *isaacCTX;
	unsigned char **inputs;
	int i, j, k;

	int numBlocks = 100, blockSize = 1000, baseBlockSize = 1000;


	isaacCTX = (randctx*) calloc(1, sizeof(randctx));
	getIsaacContext(isaacCTX);

	inputs = (unsigned char **) calloc(numBlocks * 100, sizeof(unsigned char *));
	for(i = 0; i < numBlocks * 100; i ++)
	{
		inputs[i] = generateIsaacRandBytes(isaacCTX, blockSize * 100, blockSize * 100);
	}

	set_up_server_socket(destWrite, writeSocket, mainWriteSock, writePort);
	set_up_server_socket(destRead, readSocket, mainReadSock, readPort);

	for(i = 0; i < 2; i ++)
	{
		blockSize = baseBlockSize;

		for(j = 0; j < 3; j ++)
		{
			c_0 = clock();
			timestamp_0 = timestamp();

			for(k = 0; k < numBlocks; k ++)
			{
				sendBoth(writeSocket, inputs[k], blockSize);
			}

			c_1 = clock();
			timestamp_1 = timestamp();

			printf("\nSending %d blocks of size %d\n", numBlocks, blockSize);
			printf("CPU time  :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
			printf("Wall time :     %lf\n\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));
			fflush(stdout);

			blockSize *= 10;
		}
		numBlocks *= 10;
	}

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
	int commBufferLen, i, j, k;

	int numBlocks = 100, blockSize = 1000, baseBlockSize = 1000;

	isaacCTX = (randctx*) calloc(1, sizeof(randctx));
	getIsaacContext(isaacCTX);


	set_up_client_socket(readSocket, ipAddress, readPort, serv_addr_read);
	set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr_write);

	for(i = 0; i < 2; i ++)
	{
		blockSize = baseBlockSize;

		for(j = 0; j < 3; j ++)
		{
			c_0 = clock();
			timestamp_0 = timestamp();

			for(k = 0; k < numBlocks; k ++)
			{
				commBuffer = receiveBoth(readSocket, commBufferLen);
				free(commBuffer);
			}

			c_1 = clock();
			timestamp_1 = timestamp();

			printf("\nReceiving %d blocks of size %d\n", numBlocks, blockSize);
			printf("CPU time  :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
			printf("Wall time :     %lf\n\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));
			fflush(stdout);

			blockSize *= 10;
		}
		numBlocks *= 10;
	}

	close_client_socket(readSocket);
	close_client_socket(writeSocket);

	// printBytesSent();
	// printBytesReceived();
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
	int i, j, k, commBufferLen;

	int numBlocks = 100, pointsPerBlock = 100, basePointsPerBlock = 100;


	mpz_init(tempExp);
	initRandGen();
	state = seedRandGen();
	params = initBrainpool_256_Curve();
	struct eccPoint **localPreComputes = fixedBasePreComputes(params -> g, params);




	inputPoints = (struct eccPoint **) calloc(pointsPerBlock * 100, sizeof(struct eccPoint *));
	for(i = 0; i < pointsPerBlock * 100; i ++)
	{
		mpz_urandomm(tempExp, *state, params -> n);
		inputPoints[i] = fixedPointMultiplication(localPreComputes, tempExp, params);
	}

	printf("Points generated\n");

	set_up_server_socket(destWrite, writeSocket, mainWriteSock, writePort);
	set_up_server_socket(destRead, readSocket, mainReadSock, readPort);

	for(i = 0; i < 3; i ++)
	{
		pointsPerBlock = basePointsPerBlock;

		for(j = 0; j < 3; j ++)
		{
			c_0 = clock();
			timestamp_0 = timestamp();

			for(k = 0; k < numBlocks; k ++)
			{
				commBufferLen = 0;
				commBuffer = serialise_ECC_Point_Array(inputPoints, pointsPerBlock, &commBufferLen);
				sendBoth(writeSocket, commBuffer, commBufferLen);
				free(commBuffer);
			}

			c_1 = clock();
			timestamp_1 = timestamp();

			printf("\nSending %d blocks of %d points\n", numBlocks, pointsPerBlock);
			printf("CPU time  :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
			printf("Wall time :     %lf\n\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));

			pointsPerBlock *= 10;
		}

		numBlocks *= 10;
	}

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
	int commBufferLen, bufferOffset = 0, arrayLen = 0, i, j, k;

	int numBlocks = 100, pointsPerBlock = 100, basePointsPerBlock = 100;


	set_up_client_socket(readSocket, ipAddress, readPort, serv_addr_read);
	set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr_write);

	for(i = 0; i < 3; i ++)
	{
		pointsPerBlock = basePointsPerBlock;

		for(j = 0; j < 3; j ++)
		{

			c_0 = clock();
			timestamp_0 = timestamp();

			for(k = 0; k < numBlocks; k ++)
			{
				bufferOffset = 0;
				arrayLen = 0;
				commBuffer = receiveBoth(readSocket, commBufferLen);
				outputPoints = deserialise_ECC_Point_Array(commBuffer, &arrayLen, &bufferOffset);
				free(commBuffer);
			}

			c_1 = clock();
			timestamp_1 = timestamp();

			printf("\nReceiving %d blocks of %d points\n", numBlocks, pointsPerBlock);
			printf("CPU time  :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
			printf("Wall time :     %lf\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));
			
			pointsPerBlock *= 10;
		}
		numBlocks *= 10;
	}



	close_client_socket(readSocket);
	close_client_socket(writeSocket);
}
