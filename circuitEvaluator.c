#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


#include "circuits/circuitUtils.h"

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
		runExecutor_L_2013_CnC_OT(rawInputCircuit, startOfInputChain, ipAddress, portNumStr, globalIsaacContext);
		// runExecutor_LP_2010_CnC_OT(rawInputCircuit, startOfInputChain, ipAddress, portNumStr, globalIsaacContext);
		// testReceive_OT_PVW_ECC(ipAddress);
		// test_ZKPoK_ExtDH_Tuple_Prover(ipAddress);
		// test_CnC_OT_Mod_Receiver(ipAddress);
	}
	else
	{
		printf("Running Builder.\n");
		// runBuilder_SH(circuitFilepath, startOfInputChain, portNumStr, globalIsaacContext);
		runBuilder_L_2013_CnC_OT(rawInputCircuit, startOfInputChain, portNumStr, globalIsaacContext);
		// runBuilder_LP_2010_CnC_OT(rawInputCircuit, startOfInputChain, portNumStr, globalIsaacContext);
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



void testHKE(char *circuitFilepath, char *ipAddress, char *portNumStr, char *inputFilename, int builder)
{
	struct timespec timestamp_0, timestamp_1;
	clock_t c_0, c_1;
	struct RawCircuit *rawInputCircuit = readInCircuit_Raw(circuitFilepath);
	struct idAndValue *startOfInputChain = readInputDetailsFile_Alt(inputFilename);
	struct eccParams *params = initBrainpool_256_Curve();

	struct eccPoint ***NaorPinkasInputs, *C;
	mpz_t **aList;
	gmp_randstate_t *state;
	int numCircuits = 1;

	struct HKE_Output_Struct_Builder *outputStruct;
	struct DDH_Group *group;

	struct idAndValue *startOfInputChainExec = readInputDetailsFile_Alt( (char*)"./inputs/adder_32bit.executor.input" );
	struct idAndValue *startOfInputChainBuilder = readInputDetailsFile_Alt( (char*)"./inputs/adder_32bit.builder.input" );

	state = seedRandGen();

	// So for now we are using non-ECC crypto for this, so yeah. Crappy security.
	group = get_128_Bit_Group(*state);
	globalIsaacContext = (randctx*) calloc(1, sizeof(randctx));
	getIsaacContext(globalIsaacContext);

	c_0 = clock();
	timestamp_0 = timestamp();


	outputStruct = getOutputSecretsAndScheme(rawInputCircuit -> numOutputs, 6, *state, group);
	C = setup_OT_NP_Sender(params, *state);
	aList = getNaorPinkasInputs(rawInputCircuit -> numInputsBuilder, numCircuits, *state, params);
	NaorPinkasInputs = computeNaorPinkasInputs(C, aList, rawInputCircuit -> numInputsBuilder, numCircuits, params);

	struct Circuit *inputCircuit = readInCircuit_FromRaw_HKE_2013(globalIsaacContext, rawInputCircuit, C, NaorPinkasInputs[0], outputStruct, 0, params, builder);


	setCircuitsInputs_Hardcode(startOfInputChainExec, inputCircuit, 0xFF);
	setCircuitsInputs_Hardcode(startOfInputChainBuilder, inputCircuit, 0xFF);

	runCircuitExec( inputCircuit, 0, 0 );
 
    printOutputHexString(inputCircuit);
}






// Useage. Circuit, IP, Port, Input file, builder flag
int main(int argc, char *argv[])
{
	srand(time(NULL));

	// testVSS();
	testHKE(argv[1], argv[2], argv[3], argv[4], atoi(argv[5]));
	// runProtocol(argv[1], argv[2], argv[3], argv[4], atoi(argv[5]));
	// testECC_Utils();
	// testCircuitComp(argv[1]);

	return 0;
}




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

