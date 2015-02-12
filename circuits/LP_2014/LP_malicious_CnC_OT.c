#ifndef LP_2014
#define LP_2014

void runBuilder_LP_2014_CnC_OT(char *circuitFilepath, char *inputFilepath, char *portNumStr)
{
	struct sockaddr_in destWrite, destRead;
	int writeSocket, readSocket, mainWriteSock, mainReadSock;
	int writePort = atoi(portNumStr), readPort = writePort + 1;

	struct Circuit **circuitsArray;
	int i, secParam_Circuits = 1;

	circuitsArray = (struct Circuit **) calloc(secParam_Circuits, sizeof(struct Circuit*));


	set_up_server_socket(destWrite, writeSocket, mainWriteSock, writePort);
	set_up_server_socket(destRead, readSocket, mainReadSock, readPort);

	printf("Executor has connected to us.\n");


	struct timespec timestamp_0 = timestamp(), timestamp_1;
	clock_t c_0, c_1;
	c_0 = clock();

	for(i = 0; i < secParam_Circuits; i++)
	{
		circuitsArray[i] = readInCircuitRTL(circuitFilepath);
	}

	for(i = 0; i < secParam_Circuits; i++)
	{
		readInputDetailsFileBuilder( inputFilepath, circuitsArray[i] -> gates );
	}

	c_1 = clock();
	timestamp_1 = timestamp();

	printTiming(&timestamp_0, &timestamp_1, c_0, c_1, "\nBuilding/Inputting all Circuits");


	for(i = 0; i < secParam_Circuits; i++)
	{
		sendCircuit(writeSocket, readSocket, circuitsArray[i]);
	}

	for(i = 0; i < secParam_Circuits; i++)
	{
		runCircuitBuilder( circuitsArray[i], writeSocket, readSocket );
	}

	close_server_socket(writeSocket, mainWriteSock);
	close_server_socket(readSocket, mainReadSock);


	for(i = 0; i < secParam_Circuits; i ++)
	{
		freeCircuitStruct(circuitsArray[i]);
	}
}



void runExecutor_LP_2014_CnC_OT(char *inputFilepath, char *ipAddress, char *portNumStr)
{
	struct sockaddr_in serv_addr_write, serv_addr_read;
	int writeSocket, readSocket;
	int readPort = atoi(portNumStr), writePort = readPort + 1;
	int i, secParam_Circuits = 1;

	struct Circuit **circuitsArray = (struct Circuit**) calloc(secParam_Circuits, sizeof(struct Circuit*));
	struct wire *tempWire;


	set_up_client_socket(readSocket, ipAddress, readPort, serv_addr_read);
	set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr_write);

	printf("Connected to builder.\n");

	for(i = 0; i < secParam_Circuits; i ++)
	{
		circuitsArray[i] = receiveFullCircuit(writeSocket, readSocket);
	}

	for(i = 0; i < secParam_Circuits; i ++)
	{
		runCircuitExec( circuitsArray[i], writeSocket, readSocket, inputFilepath );
	}


	close_client_socket(readSocket);
	close_client_socket(writeSocket);

	printMajorityOutputAsHex(circuitsArray, secParam_Circuits);

	testAES_FromRandom();

	for(i = 0; i < secParam_Circuits; i ++)
	{
		freeCircuitStruct(circuitsArray[i]);
	}
}

#endif