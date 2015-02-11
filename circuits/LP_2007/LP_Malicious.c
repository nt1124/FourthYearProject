#ifndef LP_2007
#define LP_2007

void runBuilder_LP_2007(char *circuitFilepath, char *inputFilepath, char *portNumStr)
{
	struct sockaddr_in destWrite, destRead;
	int writeSocket, readSocket, mainWriteSock, mainReadSock;
	int writePort = atoi(portNumStr), readPort = writePort + 1;

	struct Circuit **circuitsArray;
	int i, securityParam = 5;

	circuitsArray = (struct Circuit **) calloc(securityParam, sizeof(struct Circuit*));


	set_up_server_socket(destWrite, writeSocket, mainWriteSock, writePort);
	set_up_server_socket(destRead, readSocket, mainReadSock, readPort);

	printf("Executor has connected to us.\n");


	struct timespec timestamp_0 = timestamp(), timestamp_1;
	clock_t c_0, c_1;
	c_0 = clock();

	for(i = 0; i < securityParam; i++)
	{
		circuitsArray[i] = readInCircuitRTL_CnC(circuitFilepath, 1);
	}

	for(i = 0; i < securityParam; i++)
	{
		readInputDetailsFileBuilder( inputFilepath, circuitsArray[i] -> gates );
	}

	c_1 = clock();
	timestamp_1 = timestamp();

	printTiming(&timestamp_0, &timestamp_1, c_0, c_1, "\nBuilding/Inputting all Circuits");


	for(i = 0; i < securityParam; i++)
	{
		sendCircuit(writeSocket, readSocket, circuitsArray[i]);
	}

	for(i = 0; i < securityParam; i++)
	{
		runCircuitBuilder( circuitsArray[i], writeSocket, readSocket );
	}

	close_server_socket(writeSocket, mainWriteSock);
	close_server_socket(readSocket, mainReadSock);


	for(i = 0; i < securityParam; i ++)
	{
		freeCircuitStruct(circuitsArray[i]);
	}
}



void runExecutor_LP_2007(char *inputFilepath, char *ipAddress, char *portNumStr)
{
	struct sockaddr_in serv_addr_write, serv_addr_read;
	int writeSocket, readSocket;
	int readPort = atoi(portNumStr), writePort = readPort + 1;
	int i, securityParam = 5;

	struct Circuit **circuitsArray = (struct Circuit**) calloc(securityParam, sizeof(struct Circuit*));
	struct wire *tempWire;


	set_up_client_socket(readSocket, ipAddress, readPort, serv_addr_read);
	set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr_write);

	printf("Connected to builder.\n");

	for(i = 0; i < securityParam; i ++)
	{
		circuitsArray[i] = receiveFullCircuit(writeSocket, readSocket);
	}

	for(i = 0; i < securityParam; i ++)
	{
		runCircuitExec( circuitsArray[i], writeSocket, readSocket, inputFilepath );
	}


	close_client_socket(readSocket);
	close_client_socket(writeSocket);

	printMajorityOutputAsHex(circuitsArray, securityParam);

	testAES_FromRandom();

	for(i = 0; i < securityParam; i ++)
	{
		freeCircuitStruct(circuitsArray[i]);
	}
}

#endif