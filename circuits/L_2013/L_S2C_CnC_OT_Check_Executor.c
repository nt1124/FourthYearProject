struct secCompExecutorOutput *getSecCompReturnStruct_L_2013_E(struct publicInputsWithGroup *pubInputGroup, 
															struct eccPoint **builderInputs, unsigned char *J_set,
															int J_setSize, unsigned char *output)
{
	struct secCompExecutorOutput *returnStruct = (struct secCompExecutorOutput *) calloc(1, sizeof(struct secCompExecutorOutput));

	returnStruct -> pubInputGroup = pubInputGroup;
	returnStruct -> builderInputs = builderInputs;

	returnStruct -> J_set = J_set;
	returnStruct -> J_setSize = J_setSize;

	returnStruct -> output = output;

	return returnStruct;
}


unsigned char *getDeltaPrimeCompressed(struct Circuit **circuitsArray, unsigned char *J_set, int stat_SecParams)
{
	unsigned char *deltaPrime;
	struct wire *zeroCircuitWire, *iCircuitWire;
	int i, j, k, firstJ = 0;
	const int iOffset = circuitsArray[0] -> numGates - circuitsArray[0] -> numOutputs;


	while(J_set[firstJ] == 0x01)
	{
		firstJ ++;
	}

	for(i = 0; i < circuitsArray[0] -> numOutputs; i ++)
	{
		zeroCircuitWire = circuitsArray[firstJ] -> gates[i + iOffset] -> outputWire;
		for(j = firstJ + 1; j < stat_SecParams; j ++)
		{
			if( 0x00 == J_set[j])
			{
				iCircuitWire = circuitsArray[j] -> gates[i + iOffset] -> outputWire;
				if(0 != memcmp(zeroCircuitWire -> wireOutputKey, iCircuitWire -> wireOutputKey, 16))
				{
					deltaPrime = (unsigned char *) calloc(16, sizeof(unsigned char));
					for(k = 0; k < 16; k ++)
					{
						deltaPrime[k] = zeroCircuitWire -> wireOutputKey[k] ^ iCircuitWire -> wireOutputKey[k];
					}

					printf("\nAha! Delta Prime triggered!  %d\n", j);

					return deltaPrime;
				}
			}
		}
	}

	return generateRandBytes(16, 16);
}


unsigned char *expandDeltaPrim(struct Circuit **circuitsArray, unsigned char *J_set, int checkStatSecParam)
{
	unsigned char *deltaPrimeComp, *deltaPrime = (unsigned char *) calloc(128, sizeof(unsigned char));
	int i;

	deltaPrimeComp = getDeltaPrimeCompressed(circuitsArray, J_set, checkStatSecParam);

	for(i = 0; i < 128; i ++)
	{
		deltaPrime[i] = getBitFromCharArray(deltaPrimeComp, i);
	}

	return deltaPrime;
}


unsigned char *expandBitString(unsigned char *inputString, int lengthOutput)
{
	unsigned char *expandedString = (unsigned char *) calloc(lengthOutput, sizeof(unsigned char));
	int i;

	for(i = 0; i < lengthOutput; i ++)
	{
		expandedString[i] = getBitFromCharArray(inputString, i);
	}

	return expandedString;
}


unsigned char *deserialiseK0sAndDelta(unsigned char *commBuffer, struct Circuit **circuitsArray, int numInputsB, int checkStatSecParam)
{
	struct wire *tempWire;
	unsigned char *delta = (unsigned char *) calloc(128, sizeof(unsigned char));
	int i, bufferOffset = 0;
	
	for(i = 0; i < checkStatSecParam; i ++)
	{
		tempWire = circuitsArray[i] -> gates[numInputsB] -> outputWire;

		tempWire -> outputGarbleKeys = (struct bitsGarbleKeys *) calloc(1, sizeof(struct bitsGarbleKeys));
		tempWire -> outputGarbleKeys -> key0 = (unsigned char *) calloc(16, sizeof(unsigned char));
		tempWire -> outputGarbleKeys -> key1 = (unsigned char *) calloc(16, sizeof(unsigned char));

		memcpy(tempWire -> outputGarbleKeys -> key0, commBuffer + bufferOffset, 16);
		bufferOffset += 16;
	}

	memcpy(delta, commBuffer + bufferOffset, 128);
	bufferOffset += 128;

	return delta;
}


void setDeltaXOR_onCircuitInputs(struct Circuit **circuitsArray, unsigned char **OT_Outputs, unsigned char inputBit,
								unsigned char *delta, unsigned char *deltaPrime, unsigned char *J_set,
								int numInputsB, int lengthDelta, int checkStatSecParam)
{
	struct wire *tempWire;
	int i, j, k, iOffset, OT_index, u_v_index;
	unsigned char **XORedInputs = (unsigned char **) calloc(checkStatSecParam, sizeof(unsigned char *));


	for(j = 0; j < checkStatSecParam; j ++)
	{
		XORedInputs[j] = (unsigned char *) calloc(16, sizeof(unsigned char));
	}

	for(i = 0; i < lengthDelta; i ++)
	{
		iOffset = checkStatSecParam * i;
		u_v_index = 2 * iOffset;

		for(j = 0; j < checkStatSecParam; j ++)
		{
			if(0x00 == J_set[j] && NULL != OT_Outputs[u_v_index + deltaPrime[i]])
			{
				for(k = 0; k < 16; k ++)
				{
					XORedInputs[j][k] ^= OT_Outputs[u_v_index + deltaPrime[i]][k];
				}
			}
			else if(NULL != OT_Outputs[u_v_index + delta[i]])
			{
				for(k = 0; k < 16; k ++)
				{
					XORedInputs[j][k] ^= OT_Outputs[u_v_index + delta[i]][k];
				}
			}
			u_v_index += 2;
		}
	}

	for(j = 0; j < checkStatSecParam; j ++)
	{
		tempWire = circuitsArray[j] -> gates[numInputsB] -> outputWire;
		memcpy(tempWire -> outputGarbleKeys -> key1, XORedInputs[j], 16);

		tempWire -> wireOutputKey = (unsigned char *) calloc(16, sizeof(unsigned char));

		if(0x00 == inputBit)
		{
			memcpy(tempWire -> wireOutputKey, tempWire -> outputGarbleKeys -> key0, 16);
			tempWire -> wirePermedValue = (0x01 & tempWire -> wirePerm);
		}
		else
		{
			memcpy(tempWire -> wireOutputKey, tempWire -> outputGarbleKeys -> key1, 16);
			tempWire -> wirePermedValue = 0x01 ^ (0x01 & tempWire -> wirePerm);
		}
	}
}



struct secCompExecutorOutput *SC_DetectCheatingExecutor(int writeSocket, int readSocket, struct RawCircuit *rawInputCircuit,
														unsigned char *deltaPrime, int lengthDelta,
														int checkStatSecParam, gmp_randstate_t *state )
{
	struct Circuit **circuitsArray = (struct Circuit **) calloc(checkStatSecParam, sizeof(struct Circuit*));
	struct publicInputsWithGroup *pubInputGroup;
	struct eccParams *params;
	struct idAndValue *startOfInputChain = convertArrayToChain(deltaPrime, lengthDelta, 0);
	struct revealedCheckSecrets *secretsRevealed;
	struct eccPoint **builderInputs;
	struct secCompExecutorOutput *returnStruct;

	unsigned char *commBuffer, *J_set, **OT_Outputs, *output, *delta, inputBit = 0x00;
	int commBufferLen = 0, i, J_setSize = 0, arrayLen = 0, circuitsChecked = 0;

	struct timespec int_t_0, int_t_1;
	clock_t int_c_0, int_c_1;

	struct params_CnC_ECC *OT_params_R;
	struct otKeyPair_ECC **OT_keyPairs_R;

	// Zero the sub-computation bytes sent/received counters.
	zeroBothSubCounters();

	params = initBrainpool_256_Curve();


	int_t_0 = timestamp();
	int_c_0 = clock();

	// deltaPrime = (unsigned char *) calloc(128, sizeof(unsigned char));
	OT_params_R = OT_CnC_Receiver_Setup_Params(lengthDelta, state, deltaPrime,
											checkStatSecParam, 1024);
	OT_keyPairs_R = OT_CnC_Receiver_Produce_Queries(OT_params_R, lengthDelta,
													state, deltaPrime, checkStatSecParam);
	J_set = OT_params_R -> crs -> J_set;

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "subOT - Receiver Prep");
	printZeroBothSubCounters();


	receiveInt(readSocket);

	int_t_0 = timestamp();
	int_c_0 = clock();

	OT_CnC_Receiver_Send_Queries(writeSocket, readSocket, OT_params_R, OT_keyPairs_R, lengthDelta,
								state, deltaPrime, checkStatSecParam);
	OT_Outputs = OT_CnC_Receiver_Transfer(writeSocket, readSocket, OT_params_R, OT_keyPairs_R,
									lengthDelta, state, deltaPrime, checkStatSecParam);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "subOT - Receiver Transfer");
	printZeroBothSubCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	pubInputGroup = receivePublicCommitments(writeSocket, readSocket);

	for(i = 0; i < checkStatSecParam; i ++)
	{
		circuitsArray[i] = receiveFullCircuit(writeSocket, readSocket);
	}
	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Received Sub-circuits etc.");
	printZeroBothSubCounters();


	// OT_Inputs
	int_t_0 = timestamp();
	int_c_0 = clock();

	// THIS SHOULD ONLY BE UNCOMMENTED FOR TESTING, MEANS WE CAN GET AN EASY DELTA = DELTA'

	commBuffer = receiveBoth(readSocket, commBufferLen);
	delta = deserialiseK0sAndDelta(commBuffer, circuitsArray, rawInputCircuit -> numInputs_P1, checkStatSecParam);

	if(0 == memcmp(delta, deltaPrime, 128))
	{
		inputBit = 0x01;
	}


	setDeltaXOR_onCircuitInputs(circuitsArray, OT_Outputs, inputBit, delta, deltaPrime, J_set,
								rawInputCircuit -> numInputs_P1, lengthDelta, checkStatSecParam);
	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Get Delta and k_0s.");
	printZeroBothSubCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	secretsRevealed = executor_decommitToJ_Set(writeSocket, readSocket, circuitsArray, pubInputGroup -> public_inputs,
							pubInputGroup -> params, J_set, &J_setSize, checkStatSecParam);

	circuitsChecked = secretInputsToCheckCircuits(circuitsArray, rawInputCircuit, pubInputGroup -> public_inputs,
												secretsRevealed -> revealedSecrets,
												secretsRevealed -> revealedCircuitSeeds, pubInputGroup -> params,
												J_set, J_setSize, checkStatSecParam);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Sub-circuit correctness and decommit to J-Set");
	printZeroBothSubCounters();

	printf("\nSub-circuits Correct = %d\n", circuitsChecked);

	int_t_0 = timestamp();
	int_c_0 = clock();

	// Receive the points representing the Builder's input to the evaluation circuits
	commBuffer = receiveBoth(readSocket, commBufferLen);
	commBufferLen = 0;
	builderInputs = deserialise_ECC_Point_Array(commBuffer, &arrayLen, &commBufferLen);
	free(commBuffer);

	// And the permuted input bits for the builder on the evaluation circuits.
	commBuffer = receiveBoth(readSocket, commBufferLen);
	setBuilderInputs(builderInputs, commBuffer, J_set, J_setSize, circuitsArray,
					pubInputGroup -> public_inputs, pubInputGroup -> params);
	free(commBuffer);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Sub-circuit get builder inputs.");
	printZeroBothSubCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	printf("\nEvaluating Circuits ");
	for(i = 0; i < checkStatSecParam; i ++)
	{
		if(0x00 == J_set[i])
		{
			printf("%d, ", i);
			fflush(stdout);
			runCircuitExec( circuitsArray[i], writeSocket, readSocket );
		}
	}
	printf("\n");

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Evaluate Sub-circuits.");
	printZeroBothSubCounters();


	output = getMajorityOutput(circuitsArray, checkStatSecParam, J_set);
	if(inputBit == 0)
	{
		output = NULL;
	}

	for(i = 0; i < checkStatSecParam; i ++)
	{
		freeCircuitStruct(circuitsArray[i], 0);
	}

	returnStruct = getSecCompReturnStruct_L_2013_E(pubInputGroup, builderInputs, J_set, J_setSize, output);

	return returnStruct;
}

