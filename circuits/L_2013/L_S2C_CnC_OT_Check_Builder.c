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
	}

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

	return OT_Inputs;
}


unsigned char *getK0_AndDelta(struct Circuit **circuitsArray, unsigned char *delta, int numInputsB,
							int checkStatSecParam, int *commBufferLen)
{
	struct wire *tempWire;
	unsigned char *commBuffer = (unsigned char *) calloc(16 * (checkStatSecParam) + 128, sizeof(unsigned char));
	int bufferOffset = 0, i;

	for(i = 0; i < checkStatSecParam; i ++)
	{
		tempWire = circuitsArray[i] -> gates[numInputsB] -> outputWire;
		memcpy(commBuffer + bufferOffset, tempWire -> outputGarbleKeys -> key0, 16);
		bufferOffset += 16;
	}

	memcpy(commBuffer + bufferOffset, delta, 128);
	bufferOffset += 128;

	*commBufferLen = bufferOffset;

	return commBuffer;
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

	struct timespec int_t_0, int_t_1;
	clock_t int_c_0, int_c_1;


	params = initBrainpool_256_Curve();
	public_inputs = computePublicInputs(secret_inputs, params);
	seedList = generateRandUintList(checkStatSecParam + 1);
	circuitsArray = buildAllCircuits(rawInputCircuit, startOfInputChain, *state, checkStatSecParam, seedList, params, secret_inputs, public_inputs);


	sendPublicCommitments(writeSocket, readSocket, public_inputs, params);
	for(i = 0; i < checkStatSecParam; i++)
	{
		sendCircuit(writeSocket, readSocket, circuitsArray[i]);
	}

	OT_Inputs = getCheckCircuitOT_Inputs(circuitsArray, delta, checkStatSecParam, lengthDelta);


	int_t_0 = timestamp();
	int_c_0 = clock();

	printf("BUILDER IS SENDING THE OTs\n");
	full_CnC_OT_Sender_ECC(writeSocket, readSocket, lengthDelta,
						OT_Inputs, state, checkStatSecParam, 1024);

	commBuffer = getK0_AndDelta(circuitsArray, delta, rawInputCircuit -> numInputsBuilder, checkStatSecParam, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "subOT - Sender");

	// So yeah, this freeing causes crashes later, for some reason.
	// Sucks :P
	/*
	for(i = 0; i < checkStatSecParam; i ++)
	{
		freeCircuitStruct(circuitsArray[i], 0);
	}
	*/

	return output;
}






