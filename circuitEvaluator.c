#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


#include "circuits/circuitUtils.h"
#include "benchmarking.c"

randctx *globalIsaacContext;




void runProtocol(char *circuitFilepath, char *ipAddress, char *portNumStr, char *inputFilename, int builder)
{
	struct timespec timestamp_0, timestamp_1;
	clock_t c_0, c_1;
	struct RawCircuit *rawInputCircuit = readInCircuit_Raw(circuitFilepath);
	struct idAndValue *startOfInputChain = readInputDetailsFile_Alt(inputFilename);
	struct eccParams *params = initBrainpool_256_Curve();


	globalIsaacContext = (randctx*) calloc(1, sizeof(randctx));
	getIsaacContext(globalIsaacContext);

	c_0 = clock();
	timestamp_0 = timestamp();


	if(0 == builder)
	{
		printf("Running Executor.\n");
		// runExecutor_SH(startOfInputChain, ipAddress, portNumStr, globalIsaacContext);
		// runExecutor_L_2013_CnC_OT(rawInputCircuit, startOfInputChain, ipAddress, portNumStr, globalIsaacContext);
		// runExecutor_L_2013_HKE(rawInputCircuit, startOfInputChain, ipAddress, portNumStr, globalIsaacContext);
		// runP1_HKE_2013(rawInputCircuit, startOfInputChain, portNumStr, globalIsaacContext);
		runExecutor_LP_2010_CnC_OT(rawInputCircuit, startOfInputChain, ipAddress, portNumStr, globalIsaacContext);
	}
	else
	{
		printf("Running Builder.\n");
		// runBuilder_SH(circuitFilepath, startOfInputChain, portNumStr, globalIsaacContext);
		// runBuilder_L_2013_CnC_OT(rawInputCircuit, startOfInputChain, portNumStr, globalIsaacContext);
		// runBuilder_L_2013_HKE(rawInputCircuit, startOfInputChain, portNumStr, globalIsaacContext);
		// runP2_HKE_2013(rawInputCircuit, startOfInputChain, ipAddress, portNumStr, globalIsaacContext);
		runBuilder_LP_2010_CnC_OT(rawInputCircuit, startOfInputChain, portNumStr, globalIsaacContext);
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
	srand(time(NULL));

	// elgamal_commit_main();
	// testHKE(argv[1], argv[2], argv[3], argv[4], atoi(argv[5]));
	runProtocol(argv[1], argv[2], argv[3], argv[4], atoi(argv[5]));
	// testCircuitComp(argv[1]);
	// test_local_OT_NP(atoi(argv[5]));
	// testVSS();
	// test_ZKPoK_ECC();

	return 0;
}




// elgamal_commit_main();
// runLocally(circuitFilepath, argv[2], argv[3]);
// runProtocol(argv[1], argv[2], argv[3], argv[4], atoi(argv[5]));
// testRunZeroedInput(circuitFilepath);
// testRawCircuits(argv[1], argv[2], argv[3]);
// test_ZKPoK();
// test_ZKPoK_ECC();
// test_ZKPoK_ECC_1Of2();
// testECC_Utils();
// testOT_PWV_DDH_Local_ECC();
// test_local_CnC_OT();
// test_local_CnC_OT_ECC();

