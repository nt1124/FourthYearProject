unsigned char ***getCheckCircuitOT_Inputs(struct Circuit **circuitsArray, unsigned char *delta,
										int checkStatSecParam, int lengthDelta)
{
	unsigned char ***OT_Inputs = (unsigned char ***) calloc(2, sizeof(unsigned char**));
	unsigned char goodBit, *XOR_Sum = (unsigned char *) calloc(lengthDelta, sizeof(unsigned char));
	int i, j, k, OT_Index = 0;


	OT_Inputs[0] = (unsigned char **) calloc(lengthDelta * checkStatSecParam, sizeof(unsigned char*));
	OT_Inputs[1] = (unsigned char **) calloc(lengthDelta * checkStatSecParam, sizeof(unsigned char*));

	for(i = 0; i < checkStatSecParam; i ++)
	{
		// Change this, we want to COPY the relevant k1 in into the final OT_Inputs for the round
		// Then we just need to XOR this with each of the other good keys. 
		memset(XOR_Sum, 0x00, lengthDelta);
		for(j = 0; j < lengthDelta - 1; j ++)
		{
			goodBit = delta[j];

			OT_Inputs[0][OT_Index] = generateRandBytes(lengthDelta, lengthDelta);
			OT_Inputs[1][OT_Index] = generateRandBytes(lengthDelta, lengthDelta);

			for(k = 0; k < lengthDelta; k ++)
			{

			}

			OT_Index ++;
		}
	}

	return OT_Inputs;
}



unsigned char *SC_DetectCheatingBuilder(int writeSocket, int readSocket,
										struct RawCircuit *rawInputCircuit,
										struct idAndValue *startOfInputChain,
										struct secret_builderPRS_Keys *secret_inputs,
										int checkStatSecParam,
										gmp_randstate_t *state )
{
	struct Circuit **circuitsArray;
	struct public_builderPRS_Keys *public_inputs;
	struct eccParams *params;

	unsigned char *commBuffer, *J_set, ***OT_Inputs, *output;
	unsigned int *seedList;
	int commBufferLen = 0, i;


	params = initBrainpool_256_Curve();
	public_inputs = computePublicInputs(secret_inputs, params);
	seedList = generateRandUintList(checkStatSecParam + 1);
	circuitsArray = buildAllCircuits(rawInputCircuit, startOfInputChain, *state, checkStatSecParam, seedList, params, secret_inputs, public_inputs);

	sendPublicCommitments(writeSocket, readSocket, public_inputs, params);
	for(i = 0; i < stat_SecParam; i++)
	{
		sendCircuit(writeSocket, readSocket, circuitsArray[i]);
	}



	for(i = 0; i < stat_SecParam; i ++)
	{
		freeCircuitStruct(circuitsArray[i], 0);
	}

	return output;
}






