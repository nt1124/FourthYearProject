#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
// #include <thread>

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
	struct RawCircuit *rawInputCircuit = readInCircuit_Raw(circuitFilepath);
	struct idAndValue *startOfInputChain = readInputDetailsFile_Alt(inputFilename);

	if(0 == builder)
	{
		printf("Running Executor.\n");
		// runExecutor_SH(startOfInputChain, ipAddress, portNumStr);
		runExecutor_L_2013_CnC_OT(rawInputCircuit, startOfInputChain, ipAddress, portNumStr);
		// runExecutor_LP_2010_CnC_OT(rawInputCircuit, startOfInputChain, ipAddress, portNumStr);
		// testReceive_OT_PVW_ECC(ipAddress);
		// test_ZKPoK_ExtDH_Tuple_Prover(ipAddress);
		// test_CnC_OT_Mod_Receiver(ipAddress);
	}
	else
	{
		printf("Running Builder.\n");
		// runBuilder_SH(circuitFilepath, startOfInputChain, portNumStr);
		runBuilder_L_2013_CnC_OT(rawInputCircuit, startOfInputChain, portNumStr);
		// runBuilder_LP_2010_CnC_OT(rawInputCircuit, startOfInputChain, portNumStr);
		// testSender_OT_PVW_ECC();
		// test_ZKPoK_ExtDH_Tuple_Verifier();
		// test_CnC_OT_Mod_Sender();
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

	runProtocol(argv[1], argv[2], argv[3], argv[4], atoi(argv[5]));
	// test_ZKPoK();
	// test_ZKPoK_ECC();
	// test_ZKPoK_ECC_1Of2();
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
