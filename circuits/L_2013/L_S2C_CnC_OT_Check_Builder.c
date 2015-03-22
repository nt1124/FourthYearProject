unsigned char ***getCheckCircuitOT_Inputs(struct Circuit **circuitsArray, unsigned char *delta,
										int checkStatSecParam, int lengthDelta)
{
	struct wire *tempWire;
	unsigned char ***OT_Inputs = (unsigned char ***) calloc(2, sizeof(unsigned char**));
	unsigned char goodBit, **XOR_Sum = (unsigned char **) calloc(checkStatSecParam, sizeof(unsigned char *));
	int i, j, k, OT_Index = 0, numInputsB;


	numInputsB = circuitsArray[0] -> numInputsBuilder;
	OT_Inputs[0] = (unsigned char **) calloc(lengthDelta * checkStatSecParam, sizeof(unsigned char*));
	OT_Inputs[1] = (unsigned char **) calloc(lengthDelta * checkStatSecParam, sizeof(unsigned char*));

	for(i = 0; i < checkStatSecParam; i ++)
	{
		XOR_Sum[i] = (unsigned char *) calloc(16, sizeof(unsigned char));
		tempWire = circuitsArray[i] -> gates[numInputsB] -> outputWire;
		memcpy(XOR_Sum[i], tempWire -> outputGarbleKeys -> key1, 16);

		printf("\n%03d --  ", i);
		for(k = 0; k < 16; k ++)
		{
			printf("%02X", circuitsArray[i] -> gates[numInputsB] -> outputWire -> outputGarbleKeys -> key1[k]);
		}
	}
	printf("\n");

	for(i = 0; i < lengthDelta - 1; i ++)
	{
		goodBit = getBitFromCharArray(delta, i);

		for(j = 0; j < checkStatSecParam; j ++)
		{
			OT_Inputs[0][OT_Index] = generateRandBytes(16, 16);
			OT_Inputs[1][OT_Index] = generateRandBytes(16, 16);

			for(k = 0; k < 16; k ++)
			{
				XOR_Sum[j][k] ^= OT_Inputs[goodBit][OT_Index][k];
			}

			OT_Index ++;
		}
	}

	goodBit = getBitFromCharArray(delta, lengthDelta - 1);
	for(j = 0; j < checkStatSecParam; j ++)
	{
		OT_Inputs[goodBit][OT_Index] = (unsigned char *) calloc(16, sizeof(unsigned char));
		memcpy(OT_Inputs[goodBit][OT_Index], XOR_Sum[j], 16);

		OT_Inputs[1 - goodBit][OT_Index] = generateRandBytes(16, 16);
		OT_Index ++;
	}
	printf("\n");

	return OT_Inputs;
}



unsigned char *SC_DetectCheatingBuilder(int writeSocket, int readSocket, struct RawCircuit *rawInputCircuit,
										struct idAndValue *startOfInputChain, unsigned char *delta, int lengthDelta,
										struct secret_builderPRS_Keys *secret_inputs,
										int checkStatSecParam, gmp_randstate_t *state)
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
	for(i = 0; i < checkStatSecParam; i++)
	{
		sendCircuit(writeSocket, readSocket, circuitsArray[i]);
	}

	delta = (unsigned char *) calloc(lengthDelta, sizeof(unsigned char));
	OT_Inputs = getCheckCircuitOT_Inputs(circuitsArray, delta, checkStatSecParam, lengthDelta);
	printf("BUILDER IS SENDING THE OTs\n");
	full_CnC_OT_Sender_ECC(writeSocket, readSocket, lengthDelta,
						OT_Inputs, state, checkStatSecParam, 1024);

	// So yeah, this freeing causes crashes later, for some reason.
	/*
	for(i = 0; i < checkStatSecParam; i ++)
	{
		freeCircuitStruct(circuitsArray[i], 0);
	}
	*/

	return output;
}






