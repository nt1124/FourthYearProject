#ifndef SH_CIRCUIT
#define SH_CIRCUIT

void runBuilder_SH(char *circuitFilepath, struct idAndValue *startOfInputChain, char *portNumStr)
{
    struct sockaddr_in destWrite, destRead;
    int writeSocket, readSocket, mainWriteSock, mainReadSock;
    int writePort = atoi(portNumStr), readPort = writePort + 1;

    struct timespec timestamp_0, timestamp_1, ext_t_0, ext_t_1;
    clock_t c_0, c_1, ext_c_0, ext_c_1;

    struct Circuit *inputCircuit;
    int i;

    set_up_server_socket(destWrite, writeSocket, mainWriteSock, writePort);
    set_up_server_socket(destRead, readSocket, mainReadSock, readPort);

    printf("Executor has connected to us.\n");

    ext_t_0 = timestamp();
    ext_c_0 = clock();

    timestamp_0 = timestamp();
    c_0 = clock();

    inputCircuit = readInCircuitRTL(circuitFilepath);

    c_1 = clock();
    timestamp_1 = timestamp();

    printTiming(&timestamp_0, &timestamp_1, c_0, c_1, "\nBuilding all Circuits");


    // startOfInputChain = readInputDetailsFile_Alt(inputFilepath);
    setCircuitsInputs_Hardcode(startOfInputChain, inputCircuit, 0xFF);
    free_idAndValueChain(startOfInputChain);

    printf("Ready to send circuit.\n");
    sendCircuit(writeSocket, readSocket, inputCircuit);


    runCircuitBuilder( inputCircuit, writeSocket, readSocket );

    ext_t_1 = timestamp();
    ext_c_1 = clock();
    printTiming(&ext_t_0, &ext_t_1, ext_c_0, ext_c_1, "Total time without connection setup");

    close_server_socket(writeSocket, mainWriteSock);
    close_server_socket(readSocket, mainReadSock);


    freeCircuitStruct(inputCircuit, 1);
}



void runExecutor_SH(struct idAndValue *startOfInputChain, char *ipAddress, char *portNumStr)
{
    struct sockaddr_in serv_addr_write, serv_addr_read;
    int writeSocket, readSocket;
    int readPort = atoi(portNumStr), writePort = readPort + 1;
    int i;

    struct timespec t_0, t_1, ext_t_0, ext_t_1;
    clock_t c_0, c_1, ext_c_0, ext_c_1;

    struct Circuit *inputCircuit;
    gmp_randstate_t *state;
    struct decParams_ECC *params;

    set_up_client_socket(readSocket, ipAddress, readPort, serv_addr_read);
    set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr_write);

    printf("Connected to builder.\n");

    ext_t_0 = timestamp();
    ext_c_0 = clock();

    inputCircuit = receiveFullCircuit(writeSocket, readSocket);

    t_0 = timestamp();
    c_0 = clock();

    // startOfInputChain = readInputDetailsFile_Alt(inputFilepath);
    setCircuitsInputs_Values(startOfInputChain, inputCircuit, 0x00);
    free_idAndValueChain(startOfInputChain);

    state = seedRandGen();

    params = receiverCRS_ECC_Syn_Dec(writeSocket, readSocket);//, 1024, *state);

    executor_side_OT_ECC(writeSocket, readSocket, params, inputCircuit, state);

    c_1 = clock();
    t_1 = timestamp();
    printTiming(&t_0, &t_1, c_0, c_1, "OT - Receiver");


    runCircuitExec( inputCircuit, writeSocket, readSocket );

    ext_t_1 = timestamp();
    ext_c_1 = clock();
    printTiming(&ext_t_0, &ext_t_1, ext_c_0, ext_c_1, "Total time without connection setup");

    close_client_socket(readSocket);
    close_client_socket(writeSocket);

    printOutputHexString(inputCircuit);

    testAES_FromRandom();

    freeCircuitStruct(inputCircuit, 1);
}


void runLocally(char *circuitFilepath, char *builderInput, char *execInput)
{
    struct Circuit *inputCircuit = readInCircuitRTL(circuitFilepath);
    int i;


    readInputDetailsFileBuilder( builderInput, inputCircuit -> gates );
    readInputDetailsFileBuilder( execInput, inputCircuit -> gates );

    readLocalExec( execInput, inputCircuit );


    runCircuitLocal( inputCircuit );

    printOutputHexString(inputCircuit);
    testAES_FromRandom();

    freeCircuitStruct(inputCircuit, 1);
}


void testRunZeroedInput(char *circuitFilepath)
{    
    struct Circuit *inputCircuit = readInCircuitRTL(circuitFilepath);
    int i;


    zeroAllInputs(inputCircuit -> gates, inputCircuit -> numGates);

    runCircuitLocal(inputCircuit);
    printOutputHexString(inputCircuit);

    freeCircuitStruct(inputCircuit, 1);
    testAES_Zeroed();
}

#endif