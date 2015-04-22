int secretInputsToCheckCircuits_HKE(struct Circuit **circuitsArray, struct RawCircuit *rawInputCircuit,
									struct public_builderPRS_Keys *public_inputs,
									mpz_t *secret_J_set, ub4 **circuitSeeds, struct eccParams *params,
									unsigned char *J_set, int J_setSize, int stat_SecParam)
{
	struct Circuit *tempGarbleCircuit;
	struct wire *tempWire;
	int i, j, temp = 0, k = 0, *idList = (int*) calloc(J_setSize, sizeof(int));
	randctx **tempCTX = (randctx **) calloc(J_setSize, sizeof(randctx*));
	struct eccPoint ***consistentInputs;

	for(j = 0; j < stat_SecParam; j ++)
	{
		if(0x01 == J_set[j])
		{
			tempCTX[k] = (randctx*) calloc(1, sizeof(randctx));
			setIsaacContextFromSeed(tempCTX[k], circuitSeeds[j]);
			idList[k] = j;
			k ++;
		}
	}

	consistentInputs = getAllConsistentInputsAsPoints(secret_J_set, public_inputs -> public_keyPairs, params, J_set, stat_SecParam, rawInputCircuit -> numInputs_P1);


	#pragma omp parallel for default(shared) private(i, j, k, tempWire, tempGarbleCircuit) reduction(|:temp) 
	for(j = 0; j < J_setSize; j ++)
	{
		k = idList[j];


		for(i = 0; i < rawInputCircuit -> numInputs_P1; i ++)
		{
			tempWire = circuitsArray[k] -> gates[i] -> outputWire;
			tempWire -> outputGarbleKeys = (struct bitsGarbleKeys*) calloc(1, sizeof(struct bitsGarbleKeys));

			tempWire -> outputGarbleKeys -> key0 = hashECC_Point(consistentInputs[k][2 * i], 16);
			tempWire -> outputGarbleKeys -> key1 = hashECC_Point(consistentInputs[k][2 * i + 1], 16);
		}

		/* tempGarbleCircuit = readInCircuit_FromRaw_Seeded_ConsistentInput(tempCTX[j], rawInputCircuit, secret_J_set[k], public_inputs, k, params);
		tempGarbleCircuit = readInCircuit_FromRaw_HKE_2013(tempCTX[j], rawInputCircuit, builderInputs[k], outputKeysLocals, params, 1);

		temp |= compareCircuit(rawInputCircuit, circuitsArray[k], tempGarbleCircuit);

		freeTempGarbleCircuit(tempGarbleCircuit);
		*/
	}


	return temp;
}



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

	int_t_0 = timestamp();
	int_c_0 = clock();
	secretsRevealed = executor_decommitToJ_Set(writeSocket, readSocket, circuitsArray, pubInputGroup -> public_inputs,
							pubInputGroup -> params, J_set, &J_setSize, checkStatSecParam);

	printf("Stuff\n");
	fflush(stdout);

	circuitsChecked = secretInputsToCheckCircuits_HKE(circuitsArray, rawInputCircuit, pubInputGroup -> public_inputs,
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

	/*
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

