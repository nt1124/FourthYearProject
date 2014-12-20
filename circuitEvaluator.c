#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "circuitUtils.h"


void runBuilder(char *circuitFilepath, char *inputFilepath, char *portNumStr)
{
    struct sockaddr_in destWrite, destRead;
    int *execOrder;
    int writeSocket, readSocket, mainWriteSock, mainReadSock;
    int writePort = atoi(portNumStr), readPort = writePort + 1;
    int numGates, i, nLength;

    set_up_server_socket(destWrite, writeSocket, mainWriteSock, writePort);
    set_up_server_socket(destRead, readSocket, mainReadSock, readPort);

    printf("Executor has connected to us.\n");

    // struct gateOrWire **inputCircuit = readInCircuitRTL(circuitFilepath, &numGates, &execOrder);
    struct Circuit *inputCircuit = (struct Circuit*) calloc(1, sizeof(struct Circuit));

    inputCircuit = readInCircuitRTL(circuitFilepath);
    
    readInputDetailsFileBuilder( inputFilepath, inputCircuit -> gates );

    printf("Ready to send circuit.\n");
    sendCircuit(writeSocket, readSocket, inputCircuit -> gates, inputCircuit -> numGates, inputCircuit -> execOrder);

    runCircuitBuilder( inputCircuit -> gates, inputCircuit -> numGates, writeSocket, readSocket );

    close_server_socket(writeSocket, mainWriteSock);
    close_server_socket(readSocket, mainReadSock);

    
    freeCircuitStruct(inputCircuit);
}



void runExecutor(char *inputFilepath, char *ipAddress, char *portNumStr)
{
    struct sockaddr_in serv_addr_write, serv_addr_read;
    int writeSocket, readSocket;
    int readPort = atoi(portNumStr), writePort = readPort + 1;
    int numGates = 0, numOutputs = 0;
    int i;

    // struct gateOrWire **inputCircuit;
    struct Circuit *inputCircuit = (struct Circuit*) calloc(1, sizeof(struct Circuit));
    unsigned char *output;

    set_up_client_socket(readSocket, ipAddress, readPort, serv_addr_read);
    set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr_write);

    printf("Connected to builder.\n");

    inputCircuit -> numGates = receiveNumGates(writeSocket, readSocket);
    inputCircuit -> execOrder = receiveExecOrder(writeSocket, readSocket, inputCircuit -> numGates);
    inputCircuit -> gates = receiveCircuit(inputCircuit -> numGates, writeSocket, readSocket);
    printf("Received circuit.\n");

    runCircuitExec( inputCircuit -> gates, inputCircuit -> numGates, writeSocket, readSocket, inputFilepath, inputCircuit -> execOrder );


    close_client_socket(readSocket);
    close_client_socket(writeSocket);

    outputAsHexString(inputCircuit);// -> gates, inputCircuit -> numGates, &numOutputs);

    testAES_Zeroed();

    freeCircuitStruct(inputCircuit);
}


void runLocally(char *circuitFilepath, char *builderInput, char *execInput)
{
    int *execOrder = NULL;
    int numGates;
    struct Circuit *inputCircuit = readInCircuitRTL(circuitFilepath);
    int i;

    readInputDetailsFileBuilder( builderInput, inputCircuit -> gates );
    readInputDetailsFileBuilder( execInput, inputCircuit -> gates );

    runCircuitLocal( inputCircuit );// -> gates, inputCircuit -> numGates, inputCircuit -> execOrder );
    printAllOutput(inputCircuit -> gates, inputCircuit -> numGates);

    freeCircuitStruct(inputCircuit);
}


void testRunZeroedInput(char *circuitFilepath)
{
    int *execOrder = NULL;
    int numGates, i, numOutputs;
    struct Circuit *inputCircuit = readInCircuitRTL(circuitFilepath);
    unsigned char *output;
    
    zeroAllInputs(inputCircuit -> gates, inputCircuit -> numGates);

    runCircuitLocal( inputCircuit );//, numGates, execOrder );
    outputAsHexString(inputCircuit);//, numGates, &numOutputs);

    freeCircuitStruct(inputCircuit);
    testAES_Zeroed();
}


void testRTL_Read(char *circuitFilepath, char *inputFile)
{
    int *execOrder;
    int numGates, i;
    struct Circuit *inputCircuit = readInCircuitRTL(circuitFilepath);

    readInputDetailsFileBuilder( inputFile, inputCircuit -> gates );

    printf("--++  %d  ++--\n", inputCircuit -> numGates);
    for(i = 0; i < inputCircuit -> numGates; i ++)
    {
        printGateOrWire(inputCircuit -> gates[i]);
        printf("\n");
    }
}


void testRun(char *circuitFilepath, char *ipAddress, char *portNumStr, char *inputFilename, int builder)
{
    if(0 == builder)
    {
        printf("Running Executor.\n");
        // testSender_OT_SH_RSA(portNumStr);
        runExecutor(inputFilename, ipAddress, portNumStr);
    }
    else
    {
        printf("Running Builder.\n");
        // testReceiver_OT_SH_RSA(portNumStr);
        runBuilder(circuitFilepath, inputFilename, portNumStr);
    }
}

// Useage. Circuit, IP, Port, Input file, builder flag
int main(int argc, char *argv[])
{
    srand( time(NULL) );

    char *circuitFilepath = argv[1];
    int builder = atoi(argv[5]);

    // runLocally(circuitFilepath, argv[2], argv[3]);
    // testRTL_Read(circuitFilepath, argv[2]);
    testRun(circuitFilepath, argv[2], argv[3], argv[4], builder);
    // testOT_PWV_DDH_Local();
    // test_DDH_Local();
    // testRunZeroedInput(circuitFilepath);

    return 0;
}