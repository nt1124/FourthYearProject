#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <omp.h>


#include "circuits/circuitUtils.h"
#include "Benchmarking/benchmarking.h"

randctx *globalIsaacContext;




void runProtocol(char *circuitFilepath, char *ipAddress, char *portNumStr, char *inputFilename,
				int protocol, int builder)
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

	gPreComputes = fixedBasePreComputes(params -> g, params);


	if(0 == builder)
	{
		printf("Running Executor.\n");

		// runExecutor_SH(startOfInputChain, ipAddress, portNumStr, globalIsaacContext);
		if(0 == protocol)
			runExecutor_LP_2010_CnC_OT(rawInputCircuit, startOfInputChain, ipAddress, portNumStr, globalIsaacContext);
		else if(1 == protocol)
			runExecutor_L_2013_CnC_OT(rawInputCircuit, startOfInputChain, ipAddress, portNumStr, globalIsaacContext);
		else if(2 == protocol)
			runP1_HKE_2013(rawInputCircuit, startOfInputChain, portNumStr, globalIsaacContext);
		else if(3 == protocol)
			runExecutor_L_2013_HKE(rawInputCircuit, startOfInputChain, ipAddress, portNumStr, globalIsaacContext);
		else if(4 <= protocol)
		{
			benchmark(rawInputCircuit, ipAddress, portNumStr, protocol, builder);
			// benchmarkRawCommReceiver(ipAddress, portNumStr);
			// benchmark_ECC_PointReceiver(ipAddress, portNumStr);
			// benchmark_OT_LP_CnC_Receiver(ipAddress, portNumStr);
			// benchmark_OT_L_CnC_Mod_Receiver(ipAddress, portNumStr);
			// benchmark_Symm_OT_NP_Receiver(ipAddress, portNumStr);
		}
	}
	else if(1 == builder)
	{		
		printf("Running Builder.\n");
		// unsigned char *temp = (unsigned char *) calloc(72, sizeof(unsigned char));
		// startOfInputChain = convertArrayToChain(temp, 72, 0);

		// runBuilder_SH(circuitFilepath, startOfInputChain, portNumStr, globalIsaacContext);
		if(0 == protocol)
			runBuilder_LP_2010_CnC_OT(rawInputCircuit, startOfInputChain, portNumStr, globalIsaacContext);
		else if(1 == protocol)
			runBuilder_L_2013_CnC_OT(rawInputCircuit, startOfInputChain, portNumStr, globalIsaacContext);
		else if(2 == protocol)
			runP2_HKE_2013(rawInputCircuit, startOfInputChain, ipAddress, portNumStr, globalIsaacContext);		
		else if(3 == protocol)
			runBuilder_L_2013_HKE(rawInputCircuit, startOfInputChain, portNumStr, globalIsaacContext);
		else if(4 <= protocol)
		{
			benchmark(rawInputCircuit, ipAddress, portNumStr, protocol, builder);
			// benchmarkRawCommSender(portNumStr);
			// benchmark_ECC_PointSender(portNumStr);
			// benchmark_OT_LP_CnC_Sender(portNumStr);
			// benchmark_OT_L_CnC_Mod_Sender(portNumStr);
			// benchmark_Symm_OT_NP_Sender(portNumStr);
		}
	}
	else
	{
		benchmark(rawInputCircuit, ipAddress, portNumStr, protocol, builder);
		// benchmarkECC_Exponentiation();
		// benchmarkECC_Doubling();
		// benchmarkECC_GroupOp();
	}

	c_1 = clock();
	timestamp_1 = timestamp();

	printf("\nTotal Time (Includes connection open/closing)\n");
	fflush(stdout);
	printf("CPU time  :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
	printf("Wall time :     %lf\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));
}




// Useage. Circuit, IP, Port, Input file, builder flag
int main(int argc, char *argv[])
{
	srand(time(NULL));

	// elgamal_commit_main();
	// testHKE(argv[1], argv[2], argv[3], argv[4], atoi(argv[5]));
	runProtocol(argv[1], argv[2], argv[3], argv[4], atoi(argv[5]), atoi(argv[6]));
	// testCircuitComp(argv[1]);
	// test_local_OT_NP(atoi(argv[5]));
	// testVSS();
	// test_ZKPoK_ECC();
	// test_ZKPoK_ECC_1Of2();
	// testRawCheckCircuits();

	return 0;
}




// elgamal_commit_main();
// testRunZeroedInput(circuitFilepath);
// testRawCircuits(argv[1], argv[2], argv[3]);
// test_ZKPoK();
// test_ZKPoK_ECC();
// test_ZKPoK_ECC_1Of2();
// testECC_Utils();
// testOT_PWV_DDH_Local_ECC();
// test_local_CnC_OT();
// test_local_CnC_OT_ECC();

