/*
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
*/


struct secCompExecutorOutput *SC_DetectCheatingExecutor_HKE(int writeSocket, int readSocket, struct RawCircuit *rawInputCircuit,
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

	struct HKE_Output_Struct_Builder *outputStruct_Own, *outputStruct_Partner;
	struct DDH_Group *groupOwn, *groupPartner;
	struct eccPoint *C, *cTilde;

	unsigned char *commBuffer, *J_set, **OT_Outputs, *output, *delta, inputBit = 0x00;
	int commBufferLen = 0, i, J_setSize = 0, arrayLen = 0, circuitsChecked = 0;

	struct timespec int_t_0, int_t_1;
	clock_t int_c_0, int_c_1;


	params = initBrainpool_256_Curve();
	groupOwn = get_128_Bit_Group(*state);

	outputStruct_Own = getOutputSecretsAndScheme(rawInputCircuit -> numOutputs, checkStatSecParam, *state, groupOwn);
	C = setup_OT_NP_Sender(params, *state);
	cTilde = exchangeC_ForNaorPinkas(writeSocket, readSocket, C);

	pubInputGroup = receivePublicCommitments(writeSocket, readSocket);

	for(i = 0; i < checkStatSecParam; i ++)
	{
		circuitsArray[i] = receiveFullCircuit(writeSocket, readSocket);
	}


	// OT_Inputs
	int_t_0 = timestamp();
	int_c_0 = clock();

	// THIS SHOULD ONLY BE UNCOMMENTED FOR TESTING, MEANS WE CAN GET AN EASY DELTA = DELTA'
	// deltaPrime = (unsigned char *) calloc(128, sizeof(unsigned char));
	J_set = full_CnC_OT_Receiver_ECC_Alt(writeSocket, readSocket, lengthDelta,
										state, deltaPrime, &OT_Outputs, checkStatSecParam, 1024);

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
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "subOT - Receiver");

	/*
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

	printf("\nSub-circuits Correct = %d\n", circuitsChecked);

	commBuffer = receiveBoth(readSocket, commBufferLen);
	commBufferLen = 0;
	builderInputs = deserialise_ECC_Point_Array(commBuffer, &arrayLen, &commBufferLen);
	free(commBuffer);

	setBuilderInputs(builderInputs, J_set, J_setSize, circuitsArray,
					pubInputGroup -> public_inputs, pubInputGroup -> params);


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
	*/

	return returnStruct;
}

