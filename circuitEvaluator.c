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
    int sockfd, newSockFD, clilen;
    struct sockaddr_in serv_addr, cli_addr;

    sockfd = openSock();
    serv_addr = getSelfAsServer(portNumStr);
    bindAndListenToSock(sockfd, serv_addr);
    clilen = sizeof(cli_addr);
    newSockFD = acceptNextConnectOnSock(sockfd, &cli_addr, &clilen);

    printf("Executor has connected to us.\n");

	int numGates = count_lines_of_file(circuitFilepath);
	struct gateOrWire **inputCircuit = readInCircuit(circuitFilepath, numGates);
	readInputDetailsFileBuilder( inputFilepath, inputCircuit );

	printf("Ready to send circuit\n");
	sendCircuit(inputCircuit, numGates, newSockFD);
	runCircuitBuilder( inputCircuit, numGates, newSockFD );
}



void runExecutor(char *inputFilepath, char *ipAddress, char *portNumStr)
{
    int sockfd, portNum, n;
    struct sockaddr_in serv_addr;
    
    portNum = atoi(portNumStr);
    sockfd = openSock();
    serv_addr = getServerAddr(ipAddress, portNum);
    connectToServer(&sockfd, serv_addr);
    printf("Connected to builder\n");

	int numGates = 0;
	struct gateOrWire **inputCircuit;
	
	numGates = receiveNumGates(sockfd);
	inputCircuit = receiveCircuit(numGates, sockfd);
	printf("Received circuit\n");

	// Seg Faulting somewhere in here.
	runCircuitExec( inputCircuit, numGates, sockfd, inputFilepath );
	printf("Circuit has been run\n");

	printAllOutput(inputCircuit, numGates);
}



void testRun(char *circuitFilepath, int builder)
{
	char tempAlice[] = "And.alice.input";
	char tempBob[] = "And.bob.input";
	char portNumStr[] = "2345";
	char ipAddress[] = "127.0.0.1";

	if(0 == builder)
	{
		printf("Running Executor\n");
		runExecutor(tempBob, ipAddress, portNumStr);
	}
	else
	{
		printf("Running Builder\n");
		runBuilder(circuitFilepath, tempAlice, portNumStr);
	}
}


int main(int argc, char *argv[])
{
	int i, sockfd;
	srand( time(NULL) );

	char *circuitFilepath = argv[1];
	int builder = atoi(argv[2]);
	testRun(circuitFilepath, builder);
	/*

	int numGates = count_lines_of_file(circuitFilepath);

	struct gateOrWire **inputCircuit = readInCircuit(circuitFilepath, numGates);

	char tempAlice[] = "And.alice.input";
	char tempBob[] = "And.bob.input";
	readInputDetailsFileBuilder( tempAlice, inputCircuit );
	readInputDetailsFileBuilder( tempBob, inputCircuit );

	runCircuitLocal( inputCircuit, numGates );
	printAllOutput(inputCircuit, numGates);
	*/

	return 0;
}