

unsigned char *SC_DetectCheatingBuilder(int writeSocket, int readSocket,
										struct RawCircuit *rawInputCircuit,
										struct idAndValue *startOfInputChain,
										int checkStatSecParam,
										gmp_randstate_t *state )
{
	struct Circuit **circuitsArray;
	struct publicInputsWithGroup *pubInputGroup;
	struct eccParams *params;

	unsigned char *commBuffer, *J_set, ***OT_Inputs, *output;
	unsigned int *seedList;
	int commBufferLen = 0, i;


	params = initBrainpool_256_Curve();
	pubInputGroup = receivePublicCommitments(writeSocket, readSocket);

	for(i = 0; i < stat_SecParam; i ++)
	{
		circuitsArray[i] = receiveFullCircuit(writeSocket, readSocket);
	}



	for(i = 0; i < stat_SecParam; i ++)
	{
		freeCircuitStruct(circuitsArray[i], 0);
	}

	return output;
}

