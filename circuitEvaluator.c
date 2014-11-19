#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "circuitBuilder.h"
#include "circuitExecutor.h"


void printAllOutput(struct gateOrWire **inputCircuit, int numGates)
{
    int i;
    unsigned char tempBit;

    for(i = 0; i < numGates; i ++)
    {
        if( 0x0F == inputCircuit[i] -> outputWire -> wireMask )
        {
            tempBit = inputCircuit[i] -> outputWire -> wirePermedValue;
            tempBit = tempBit ^ (0x01 & inputCircuit[i] -> outputWire -> wirePerm);
            printf("Gate %d = %d\n", inputCircuit[i] -> G_ID, tempBit);
        }
    }
}


void runCircuitLocal( struct gateOrWire **inputCircuit, int numGates, int *execOrder )
{
    int i, j, gateID;

    for(i = 0; i < numGates; i ++)
    {
        gateID = execOrder[i];
        if( NULL != inputCircuit[gateID] -> gatePayload )
        {
            decryptGate(inputCircuit[gateID], inputCircuit);
        }
    }
}


void runBuilder(char *circuitFilepath, char *inputFilepath, char *portNumStr)
{
    unsigned char *nBytes;
    mpz_t *N = (mpz_t*) calloc(1, sizeof(mpz_t));
    struct sockaddr_in destWrite, destRead;
    int *execOrder;
    int writeSocket, readSocket, mainWriteSock, mainReadSock;
    int writePort = atoi(portNumStr), readPort = writePort + 1;
    int numGates, i, nLength;

    set_up_server_socket(destWrite, writeSocket, mainWriteSock, writePort);
    // set_up_server_socket(destRead, readSocket, mainReadSock, readPort);

    printf("Executor has connected to us.\n");

    struct gateOrWire **inputCircuit = readInCircuitRTL(circuitFilepath, &numGates, &execOrder);
    
    readInputDetailsFileBuilder( inputFilepath, inputCircuit );

    printf("Ready to send circuit.\n");
    sendCircuit(writeSocket, inputCircuit, numGates, execOrder);

    mpz_init(*N);
    nBytes = receiveBoth(writeSocket, nLength);
    convertBytesToMPZAlt(N, nBytes, nLength);

    runCircuitBuilder( inputCircuit, numGates, writeSocket, *N );

    close_server_socket(writeSocket, mainWriteSock);
    // close_server_socket(readSocket, mainReadSock);

    for(i = 0; i < numGates; i ++)
    {
        freeGateOrWire(inputCircuit[i]);
    }
    free(inputCircuit);
    free(N);
}



void runExecutor(char *inputFilepath, char *ipAddress, char *portNumStr)
{
    clock_t startClock = clock(); int msec;

    int *execOrder;
    int writeSocket, readSocket, writePort, readPort;
    struct sockaddr_in serv_addr_write;
    struct sockaddr_in serv_addr_read;
    int numGates = 0, i;
    struct gateOrWire **inputCircuit;

    readPort = atoi(portNumStr);
    writePort = writePort + 1;

    set_up_client_socket(readSocket, ipAddress, readPort, serv_addr_read);
    // set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr_write);

    printf("Connected to builder.\n");

    numGates = receiveNumGates(readSocket);
    execOrder = receiveExecOrder(readSocket, numGates);
    inputCircuit = receiveCircuit(numGates, readSocket);
    printf("Received circuit.\n");

    runCircuitExec( inputCircuit, numGates, readSocket, inputFilepath, execOrder );

    printAllOutput(inputCircuit, numGates);

    close_client_socket(readSocket);
    // close_client_socket(writeSocket);

    for(i = 0; i < numGates; i ++)
    {
        freeGateOrWire(inputCircuit[i]);
    }
    free(inputCircuit);

    msec = (clock() - startClock) * 1000 / CLOCKS_PER_SEC;
    printf("Total time taken %d seconds %d milliseconds\n", msec / 1000, msec % 1000);
}


void runLocally(char *circuitFilepath, char *builderInput, char *execInput)
{
    int *execOrder = NULL;
    int numGates;
    struct gateOrWire **inputCircuit = readInCircuitRTL(circuitFilepath, &numGates, &execOrder);
    int i;

    readInputDetailsFileBuilder( builderInput, inputCircuit );
    readInputDetailsFileBuilder( execInput, inputCircuit );

    runCircuitLocal( inputCircuit, numGates, execOrder );
    printAllOutput(inputCircuit, numGates);

    for(i = 0; i < numGates; i ++)
    {
        freeGateOrWire(inputCircuit[i]);
    }

    free(inputCircuit);
}


void testRunZeroedInput(char *circuitFilepath)
{
    int *execOrder = NULL;
    int numGates;
    struct gateOrWire **inputCircuit = readInCircuitRTL(circuitFilepath, &numGates, &execOrder);
    int i;

    
    zeroAllInputs(inputCircuit, numGates);

    runCircuitLocal( inputCircuit, numGates, execOrder );
    printAllOutput(inputCircuit, numGates);

    for(i = 0; i < numGates; i ++)
    {
        freeGateOrWire(inputCircuit[i]);
    }

    free(inputCircuit);

    testAES_Zeroed();
}


void testRTL_Read(char *circuitFilepath)
{
    int *execOrder;
    int numGates, i;
    struct gateOrWire **inputCircuit = readInCircuitRTL(circuitFilepath, &numGates, &execOrder);

    printf("--++  %d  ++--\n", numGates);
    for(i = 0; i < numGates; i ++)
    {
        printGateOrWire(inputCircuit[i]);
        printf("\n");
    }
}


void testRun(char *circuitFilepath, char *ipAddress, char *portNumStr, char *inputFilename, int builder)
{
    // char tempAlice[] = "And.alice.input\0";
    // char tempBob[] = "And.bob.input\0";

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
    // testRTL_Read(circuitFilepath);
    testRun(circuitFilepath, argv[2], argv[3], argv[4], builder);
    // testRunZeroedInput(circuitFilepath);

    return 0;
}