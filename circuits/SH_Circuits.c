#ifndef SH_CIRCUIT
#define SH_CIRCUIT

void runBuilder_SH(char *circuitFilepath, char *inputFilepath, char *portNumStr)
{
    struct sockaddr_in destWrite, destRead;
    int writeSocket, readSocket, mainWriteSock, mainReadSock;
    int writePort = atoi(portNumStr), readPort = writePort + 1;

    struct Circuit *inputCircuit;
    int i;

    set_up_server_socket(destWrite, writeSocket, mainWriteSock, writePort);
    set_up_server_socket(destRead, readSocket, mainReadSock, readPort);

    printf("Executor has connected to us.\n");


    struct timespec timestamp_0 = timestamp(), timestamp_1;
    clock_t c_0, c_1;
    c_0 = clock();

    inputCircuit = readInCircuitRTL_CnC(circuitFilepath, 1);

    c_1 = clock();
    timestamp_1 = timestamp();
    printf("\nBuilding all Circuits\n");
    printf("CPU time   :   %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
    printf("Wall time  :   %lf\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));


    readInputDetailsFileBuilder( inputFilepath, inputCircuit -> gates );

    printf("Ready to send circuit.\n");
    sendCircuit(writeSocket, readSocket, inputCircuit);


    runCircuitBuilder( inputCircuit, writeSocket, readSocket );


    close_server_socket(writeSocket, mainWriteSock);
    close_server_socket(readSocket, mainReadSock);


    freeCircuitStruct(inputCircuit);
}



void runExecutor_SH(char *inputFilepath, char *ipAddress, char *portNumStr)
{
    struct sockaddr_in serv_addr_write, serv_addr_read;
    int writeSocket, readSocket;
    int readPort = atoi(portNumStr), writePort = readPort + 1;
    int i;

    struct Circuit *inputCircuit;


    set_up_client_socket(readSocket, ipAddress, readPort, serv_addr_read);
    set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr_write);

    printf("Connected to builder.\n");


    inputCircuit = receiveFullCircuit(writeSocket, readSocket);

    runCircuitExec( inputCircuit, writeSocket, readSocket, inputFilepath );


    close_client_socket(readSocket);
    close_client_socket(writeSocket);

    outputAsHexString(inputCircuit);

    testAES_FromRandom();

    freeCircuitStruct(inputCircuit);
}


void runLocally(char *circuitFilepath, char *builderInput, char *execInput)
{
    struct Circuit *inputCircuit = readInCircuitRTL_CnC(circuitFilepath, 1);
    int i;


    readInputDetailsFileBuilder( builderInput, inputCircuit -> gates );
    readInputDetailsFileBuilder( execInput, inputCircuit -> gates );

    readLocalExec( execInput, inputCircuit );


    runCircuitLocal( inputCircuit );

    outputAsHexString(inputCircuit);
    testAES_FromRandom();

    freeCircuitStruct(inputCircuit);
}


void testRunZeroedInput(char *circuitFilepath)
{    
    struct Circuit *inputCircuit = readInCircuitRTL_CnC(circuitFilepath, 3);
    int i;


    zeroAllInputs(inputCircuit -> gates, inputCircuit -> numGates);

    runCircuitLocal(inputCircuit);
    outputAsHexString(inputCircuit);

    freeCircuitStruct(inputCircuit);
    testAES_Zeroed();
}

#endif