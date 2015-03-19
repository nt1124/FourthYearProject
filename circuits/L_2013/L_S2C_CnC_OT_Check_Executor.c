

unsigned char *SC_DetectCheatingExecutor(int writeSocket, int readSocket, struct RawCircuit *rawInputCircuit,
										unsigned char *deltaPrime, int lengthDelta,
										int checkStatSecParam, gmp_randstate_t *state )
{
	struct Circuit **circuitsArray = (struct Circuit **) calloc(checkStatSecParam, sizeof(struct Circuit*));
	struct publicInputsWithGroup *pubInputGroup;
	struct eccParams *params;
	// struct idAndValue *startOfInputChain = convertArrayToChain(deltaPrime, lengthDelta, 0);

	unsigned char *commBuffer, *J_set, ***OT_Inputs, *output;
	unsigned int *seedList;
	int commBufferLen = 0, i;


	params = initBrainpool_256_Curve();
	pubInputGroup = receivePublicCommitments(writeSocket, readSocket);

	for(i = 0; i < checkStatSecParam; i ++)
	{
		circuitsArray[i] = receiveFullCircuit(writeSocket, readSocket);
	}



	for(i = 0; i < checkStatSecParam; i ++)
	{
		freeCircuitStruct(circuitsArray[i], 0);
	}

	return output;
}

