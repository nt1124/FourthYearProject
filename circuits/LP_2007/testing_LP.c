#include "../circuitUtils.h"
#include "LP_utils.h"


void test_allP2_C(char *circuitFilepath, char *inputFilepath)
{
	srand( time(NULL) );
	gmp_randstate_t *state = seedRandGen();

	struct commit_batch_params *params;
	struct commit_pair_Keys **tempKeys;
	struct commit_pair_Boxes **tempBoxes;

	struct Circuit **circuitsArray;
	int i, secParam_Circuits = 2, secParam_Encoding = 2;

	unsigned char *buffer;
	int bufferSize, bufferOffset;

	params = generate_commit_params(1024, *state);
	circuitsArray = (struct Circuit **) calloc(secParam_Circuits, sizeof(struct Circuit*));

	for(i = 0; i < secParam_Circuits; i++)
	{
		circuitsArray[i] = readInCircuitRTL_CnC(circuitFilepath, secParam_Encoding);
	}

	for(i = 0; i < secParam_Circuits; i++)
	{
		readInputDetailsFileBuilder( inputFilepath, circuitsArray[i] -> gates );
	}


	bufferSize = secParam_Circuits * secParam_Encoding * circuitsArray[0] -> numInputsExecutor;
	bufferSize = getSerialSizeElgamal(params -> group, bufferSize, 2);
	buffer = (unsigned char *) calloc(bufferSize, sizeof(unsigned char));

	bufferOffset = 0;	
	tempKeys = LP_2007_commit_allP2_C(params, *state, circuitsArray, secParam_Encoding, secParam_Circuits, buffer, &bufferOffset);
	
	bufferOffset = 0;
	tempBoxes = LP_2007_commit_allP2_R(params, circuitsArray[0] -> numInputsExecutor, secParam_Encoding, secParam_Circuits, buffer, &bufferOffset);

	bufferOffset = 0;
	LP_2007_decommit_allP2_C(params, tempKeys, circuitsArray, secParam_Encoding, secParam_Circuits, buffer, &bufferOffset);


	for(i = 0; i < secParam_Circuits; i ++)
	{
		freeCircuitStruct(circuitsArray[i]);
	}

	printf("... Okay...\n");
	fflush(stdout);
}


int main(int argc, char *argv[])
{
	test_allP2_C(argv[1], argv[2]);

	return 0;
}


