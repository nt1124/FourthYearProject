#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "circuits/circuitUtils.h"




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


void runProtocol(char *circuitFilepath, char *ipAddress, char *portNumStr, char *inputFilename, int builder)
{
	struct timespec timestamp_0 = timestamp(), timestamp_1;
	clock_t c_0, c_1;
	c_0 = clock();

	if(0 == builder)
	{
		printf("Running Executor.\n");
		// runExecutor_SH(inputFilename, ipAddress, portNumStr);
		runExecutor_LP_2014_CnC_OT(circuitFilepath, inputFilename, ipAddress, portNumStr);
		// testReceive_OT_PVW_ECC(ipAddress);
	}
	else
	{
		printf("Running Builder.\n");
		// runBuilder_SH(circuitFilepath, inputFilename, portNumStr);
		runBuilder_LP_2014_CnC_OT(circuitFilepath, inputFilename, portNumStr);
		// testSender_OT_PVW_ECC();
	}

	c_1 = clock();
	timestamp_1 = timestamp();

	printf("\nTotal Time (Includes connection open/closing)\n");
	printf("CPU time  :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
	printf("Wall time :     %lf\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));
}

// Useage. Circuit, IP, Port, Input file, builder flag
int main(int argc, char *argv[])
{
	srand( time(NULL) );

	// runProtocol(argv[1], argv[2], argv[3], argv[4], atoi(argv[5]));
	// test_ZKPoK();
	test_ZKPoK_ECC();
	// testECC_Utils();
	// testOT_PWV_DDH_Local_ECC();
	// test_local_CnC_OT();
	// test_local_CnC_OT_ECC();

	return 0;
}




// runLocally(circuitFilepath, argv[2], argv[3]);
// runProtocol(argv[1], argv[2], argv[3], argv[4], atoi(argv[5]));
// testRunZeroedInput(circuitFilepath);
// testRawCircuits(argv[1], argv[2], argv[3]);
// testCircuitComp(argv[1]);