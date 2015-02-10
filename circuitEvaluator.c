#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "circuits/circuitUtils.h"
#include "circuits/SH_Circuits.c"
#include "circuits/LP_Malicious.c"



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


void runSH(char *circuitFilepath, char *ipAddress, char *portNumStr, char *inputFilename, int builder)
{
    struct timespec timestamp_0 = timestamp(), timestamp_1;
    clock_t c_0, c_1;
    c_0 = clock();

    if(0 == builder)
    {
        printf("Running Executor.\n");
        // testSender_OT_SH_RSA(portNumStr);
        // testReceive_OT_PVW(ipAddress);
        // runExecutor_SH(inputFilename, ipAddress, portNumStr);
        runExecutor_LP_2007(inputFilename, ipAddress, portNumStr);
    }
    else
    {
        printf("Running Builder.\n");
        // testReceiver_OT_SH_RSA(portNumStr);
        // testSender_OT_PVW();
        // runBuilder_SH(circuitFilepath, inputFilename, portNumStr);
        runBuilder_LP_2007(circuitFilepath, inputFilename, portNumStr);
    }

    c_1 = clock();
    timestamp_1 = timestamp();

    printf("\nTotal CPU time  :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
    printf("Total Wall time :     %lf\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));
}

// Useage. Circuit, IP, Port, Input file, builder flag
int main(int argc, char *argv[])
{
    srand( time(NULL) );

    char *circuitFilepath = argv[1];
    int builder = atoi(argv[5]);

    // runLocally(circuitFilepath, argv[2], argv[3]);
    runSH(circuitFilepath, argv[2], argv[3], argv[4], builder);
    // testRunZeroedInput(circuitFilepath);


    return 0;
}