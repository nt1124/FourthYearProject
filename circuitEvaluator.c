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

    struct Circuit *inputCircuit = (struct Circuit*) calloc(1, sizeof(struct Circuit));

    struct timespec timestamp_0 = timestamp(), timestamp_1;
    clock_t c_0, c_1;
    c_0 = clock();

    inputCircuit = readInCircuitRTL_CnC(circuitFilepath, 1);


    c_1 = clock();
    timestamp_1 = timestamp();
    double temp = seconds_timespecDiff(&timestamp_0, &timestamp_1);


    readInputDetailsFileBuilder( inputFilepath, inputCircuit -> gates );

    printf("Ready to send circuit.\n");
    sendCircuit(writeSocket, readSocket, inputCircuit);

    runCircuitBuilder( inputCircuit -> gates, inputCircuit -> numGates, writeSocket, readSocket );

    close_server_socket(writeSocket, mainWriteSock);
    close_server_socket(readSocket, mainReadSock);


    printf("\nBuilding Circuit CPU time    :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
    printf("Building Circuit Custom time :     %lf\n", temp);

    freeCircuitStruct(inputCircuit);
}



void runExecutor(char *inputFilepath, char *ipAddress, char *portNumStr)
{
    struct sockaddr_in serv_addr_write, serv_addr_read;
    int writeSocket, readSocket;
    int readPort = atoi(portNumStr), writePort = readPort + 1;
    int numGates = 0, numOutputs = 0;
    int i;

    struct Circuit *inputCircuit = (struct Circuit*) calloc(1, sizeof(struct Circuit));
    unsigned char *output, *circuitParamsBuffer = (unsigned char*) calloc(6 * sizeof(int), sizeof(unsigned char));

    set_up_client_socket(readSocket, ipAddress, readPort, serv_addr_read);
    set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr_write);

    printf("Connected to builder.\n");


    receive(readSocket, circuitParamsBuffer, 6*sizeof(int));

    memcpy(&(inputCircuit -> numGates), circuitParamsBuffer, sizeof(int));
    memcpy(&(inputCircuit -> numInputs), circuitParamsBuffer + sizeof(int), sizeof(int));
    memcpy(&(inputCircuit -> numOutputs), circuitParamsBuffer + 2 * sizeof(int), sizeof(int));
    memcpy(&(inputCircuit -> numInputsBuilder), circuitParamsBuffer + 3 * sizeof(int), sizeof(int));
    memcpy(&(inputCircuit -> numInputsExecutor), circuitParamsBuffer + 4 * sizeof(int), sizeof(int));
    memcpy(&(inputCircuit -> securityParam), circuitParamsBuffer + 5 * sizeof(int), sizeof(int));

    inputCircuit -> execOrder = receiveExecOrder(writeSocket, readSocket, inputCircuit -> numGates);
    inputCircuit -> gates = receiveCircuit(inputCircuit -> numGates, writeSocket, readSocket);

    printf("Received circuit.\n");

    /*
    for(i = 0; i < inputCircuit -> numGates; i ++)
    {
        printf("%d\n", inputCircuit -> execOrder[i]);
    }
    */

    runCircuitExec( inputCircuit, writeSocket, readSocket, inputFilepath );


    close_client_socket(readSocket);
    close_client_socket(writeSocket);

    outputAsHexString(inputCircuit);

    testAES_Zeroed();

    freeCircuitStruct(inputCircuit);
}


void runLocally(char *circuitFilepath, char *builderInput, char *execInput)
{
    int *execOrder = NULL;
    struct Circuit *inputCircuit = readInCircuitRTL_CnC(circuitFilepath, 2);
    int i, bufferLength = 0;
    unsigned char *buffer;


    readInputDetailsFileBuilder( builderInput, inputCircuit -> gates );
    // readInputDetailsFileBuilder( execInput, inputCircuit -> gates );
    printf(">>>>>>>>\n");
    fflush(stdout);
    readLocalExec( execInput, inputCircuit );
    printf(">>>>>>>>\n");
    fflush(stdout);

    /*
    buffer = serialiseCircuit(inputCircuit, &bufferLength);

    outputCircuit -> gates = deserialiseCircuit(buffer, inputCircuit -> numGates);
    */

    runCircuitLocal( inputCircuit );
    printf(">>>>>>>>\n");
    fflush(stdout);
    printAllOutput(inputCircuit);

    freeCircuitStruct(inputCircuit);
}


void testRunZeroedInput(char *circuitFilepath)
{
    int *execOrder = NULL;
    int numGates, i, numOutputs;
    struct Circuit *inputCircuit = readInCircuitRTL(circuitFilepath);
    unsigned char *output;
    
    zeroAllInputs(inputCircuit -> gates, inputCircuit -> numGates);

    runCircuitLocal( inputCircuit );
    outputAsHexString(inputCircuit);

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
        // testReceive_OT_PVW(ipAddress);
        runExecutor(inputFilename, ipAddress, portNumStr);
    }
    else
    {
        printf("Running Builder.\n");
        // testReceiver_OT_SH_RSA(portNumStr);
        // testSender_OT_PVW();
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
    testRun(circuitFilepath, argv[2], argv[3], argv[4], builder);
    // testRunZeroedInput(circuitFilepath);

    /*
    struct Circuit *inputCircuit = readInCircuitRTL_CnC(circuitFilepath, 4);

    for(builder = 0; builder < inputCircuit -> numGates; builder ++)
    {
        printGateOrWire(inputCircuit -> gates[builder]);
        printf("\n");
    }
    */

    return 0;
}