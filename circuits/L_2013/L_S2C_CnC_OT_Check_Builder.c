struct secCompBuilderOutput *getSecCompReturnStruct_L_2013_B(struct public_builderPRS_Keys *public_inputs, 
															struct eccPoint **builderInputs, unsigned char *J_set,
															int J_setSize)
{
	struct secCompBuilderOutput *returnStruct = (struct secCompBuilderOutput *) calloc(1, sizeof(struct secCompBuilderOutput));

	returnStruct -> public_inputs = public_inputs;
	returnStruct -> builderInputs = builderInputs;

	returnStruct -> J_set = J_set;
	returnStruct -> J_setSize = J_setSize;

	return returnStruct;
}


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

	for(i = 0; i < 128; i ++)
	{
		commBuffer[bufferOffset] = getBitFromCharArray(delta, i);
		bufferOffset ++;
	}


	*commBufferLen = bufferOffset;

	return commBuffer;
}



struct secCompBuilderOutput *SC_DetectCheatingBuilder(int writeSocket, int readSocket, struct RawCircuit *rawInputCircuit,
													struct idAndValue *startOfInputChain, unsigned char *delta, int lengthDelta,
													struct secret_builderPRS_Keys *secret_inputs,
													int checkStatSecParam, gmp_randstate_t *state)
{
	struct Circuit **circuitsArray;
	struct public_builderPRS_Keys *public_inputs;
	struct eccParams *params;
	struct eccPoint **builderInputs;
	struct secCompBuilderOutput *returnStruct;

	unsigned char *commBuffer, *J_set, ***OT_Inputs, *deltaExpanded;
	int commBufferLen = 0, i, J_setSize = 0, arrayLen = 0;

	struct timespec int_t_0, int_t_1;
	clock_t int_c_0, int_c_1;

	ub4 **circuitSeeds = (ub4 **) calloc(checkStatSecParam, sizeof(ub4*));
	randctx **circuitCTXs = (randctx **) calloc(checkStatSecParam, sizeof(randctx*));


	for(i = 0; i < checkStatSecParam; i ++)
	{
		circuitCTXs[i] = (randctx*) calloc(1, sizeof(randctx));
		circuitSeeds[i] = getIsaacContext(circuitCTXs[i]);
	}

	params = initBrainpool_256_Curve();
	public_inputs = computePublicInputs(secret_inputs, params);
	circuitsArray = buildAllCircuits(rawInputCircuit, startOfInputChain, *state, checkStatSecParam, params, secret_inputs, public_inputs, circuitCTXs, circuitSeeds);


	sendPublicCommitments(writeSocket, readSocket, public_inputs, params);
	for(i = 0; i < checkStatSecParam; i++)
	{
		sendCircuit(writeSocket, readSocket, circuitsArray[i]);
	}

	// THIS SHOULD ONLY BE UNCOMMENTED FOR TESTING, MEANS WE CAN GET AN EASY DELTA = DELTA'
	// delta = (unsigned char *) calloc(16, sizeof(unsigned char));
	OT_Inputs = getCheckCircuitOT_Inputs(circuitsArray, delta, checkStatSecParam, lengthDelta);


	int_t_0 = timestamp();
	int_c_0 = clock();

	full_CnC_OT_Sender_ECC(writeSocket, readSocket, lengthDelta,
						OT_Inputs, state, checkStatSecParam, 1024);

	commBuffer = getK0_AndDelta(circuitsArray, delta, rawInputCircuit -> numInputsBuilder, checkStatSecParam, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "subOT - Sender");


	J_set = builder_decommitToJ_Set(writeSocket, readSocket, circuitsArray, secret_inputs, checkStatSecParam, &J_setSize, circuitSeeds);

	builderInputs =  computeBuilderInputs(public_inputs, secret_inputs,
										J_set, J_setSize, startOfInputChain, 
										params, &arrayLen);

	commBuffer = serialise_ECC_Point_Array(builderInputs, arrayLen, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);


	returnStruct = getSecCompReturnStruct_L_2013_B(public_inputs, builderInputs, J_set, J_setSize);

	return returnStruct;
}


