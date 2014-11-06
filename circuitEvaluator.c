#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "circuitUtils.h"


int compilationOfTests()
{
	// testAES();
	// testElgamal();
	// testRSA();
	// testByteConvert();

	/*
	for(i = 0; i < numGates; i ++)
	{
		printf("+++++  Gate %02d  +++++\n", i);
		testSerialisation(inputCircuit[i]);
		printf("\n");
	}
	*/
	return 1;
}



void runBuilder(char *circuitFilepath, char *inputFilepath, char *portNumStr)
{
    int writeSocket, readSocket, mainWriteSock, mainReadSock, i;
    int writePort = atoi(portNumStr), readPort = writePort + 1;
    struct sockaddr_in serv_addr, cli_addr;

    set_up_server_socket(dest, writeSocket, mainWriteSock, writePort);
    // set_up_server_socket(dest, readSocket, mainReadSock, readPort);

    printf("Executor has connected to us.\n");

	int numGates = count_lines_of_file(circuitFilepath);
	struct gateOrWire **inputCircuit = readInCircuit(circuitFilepath, numGates);
	readInputDetailsFileBuilder( inputFilepath, inputCircuit );

	printf("Ready to send circuit.\n");
	sendCircuit(inputCircuit, numGates, writeSocket);
	runCircuitBuilder( inputCircuit, numGates, writeSocket );

    close_server_socket(writeSocket, mainWriteSock);
    // close_server_socket(readSocket, mainReadSock);

	for(i = 0; i < numGates; i ++)
	{
		freeGateOrWire(inputCircuit[i]);
	}
	free(inputCircuit);
}



void runExecutor(char *inputFilepath, char *ipAddress, char *portNumStr)
{
	clock_t startClock = clock(); int msec;

    int writeSocket, readSocket, writePort, readPort;
    struct sockaddr_in serv_addr;
	int numGates = 0, i;
	struct gateOrWire **inputCircuit;


    readPort = atoi(portNumStr);
    writePort = writePort + 1;
    
    set_up_client_socket(readSocket, ipAddress, readPort, serv_addr);
    set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr);

    printf("Connected to builder.\n");

	numGates =  receiveNumGates(sockfd);
	inputCircuit = receiveCircuit(numGates, sockfd);
	printf("Received circuit.\n");

	runCircuitExec( inputCircuit, numGates, readSocket, inputFilepath );
	printAllOutput(inputCircuit, numGates);

    close_client_socket(readSocket);
    close_client_socket(writeSocket);

	for(i = 0; i < numGates; i ++)
	{
		freeGateOrWire(inputCircuit[i]);
	}
	free(inputCircuit);

	msec = (clock() - startClock) * 1000 / CLOCKS_PER_SEC;
	printf("Time taken %d seconds %d milliseconds\n", msec / 1000, msec % 1000);
}


void runLocally(char *circuitFilepath)
{
	char tempAlice[] = "And.alice.input\0";
	char tempBob[] = "And.bob.input\0";
	int numGates = count_lines_of_file(circuitFilepath);
	struct gateOrWire **inputCircuit = readInCircuit(circuitFilepath, numGates);
	int i;


	readInputDetailsFileBuilder( tempAlice, inputCircuit );
	readInputDetailsFileBuilder( tempBob, inputCircuit );

	runCircuitLocal( inputCircuit, numGates );
	printAllOutput(inputCircuit, numGates);

	for(i = 0; i < numGates; i ++)
	{
		printf("+++++  Gate %02d  +++++\n", i);
		testSerialisation(inputCircuit[i]);
		printf("\n");
		freeGateOrWire(inputCircuit[i]);
	}

	free(inputCircuit);
}

void testRun(char *circuitFilepath, char *ipAddress, char *portNumStr, int builder)
{
	char tempAlice[] = "And.alice.input\0";
	char tempBob[] = "And.bob.input\0";

	if(0 == builder)
	{
		printf("Running Executor.\n");
		// testSender_OT_SH_RSA(portNumStr);
		runExecutor(tempBob, ipAddress, portNumStr);
	}
	else
	{
		printf("Running Builder.\n");
		// testReceiver_OT_SH_RSA(portNumStr);
		runBuilder(circuitFilepath, tempAlice, portNumStr);
	}
}


int main(int argc, char *argv[])
{
	srand( time(NULL) );

	char *circuitFilepath = argv[1];
	int builder = atoi(argv[4]);

	testRun(circuitFilepath, argv[2], argv[3], builder);

	return 0;
}