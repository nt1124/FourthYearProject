unsigned char *SC_DetectCheatingExecutor_HKE(int writeSocket, int readSocket, struct RawCircuit *rawInputCircuit,
											unsigned char *deltaPrime, int lengthDelta,
											struct eccPoint **queries_Partner,
											struct eccPoint *C, struct eccPoint *cTilde,
											int checkStatSecParam, gmp_randstate_t *state, randctx *ctx)
{
	struct Circuit **circuitsArray_Partner, **circuitsArray_Own;
	struct idAndValue *startOfInputChain;// = convertArrayToChain(deltaPrime, lengthDelta, 0);
	struct jSetRevealHKE *partnerReveals;
	struct secCompExecutorOutput *returnStruct;

	struct builderInputCommitStruct *commitStruct, *partnersCommitStruct;
	struct HKE_Output_Struct_Builder *outputStruct_Own, *outputStruct_Partner;
	struct DDH_Group *groupOwn, *groupPartner;
	struct eccParams *params;
	struct wire *tempWire;

	ub4 **circuitSeeds = (ub4 **) calloc(checkStatSecParam, sizeof(ub4*));
	randctx **circuitCTXs = (randctx **) calloc(checkStatSecParam, sizeof(randctx*));
	struct eccPoint ***NaorPinkasInputs;
	mpz_t **aList;

	unsigned char *commBuffer, *J_SetOwn, *J_SetPartner;
	unsigned char **OT_Outputs, *binaryOutput, *delta, inputBit = 0x00, ***OT_Inputs;
	int commBufferLen = 0, i, J_setSize = 0, arrayLen = 0, circuitsChecked = 0, bufferOffset = 0;
	int jSetChecks = 0, logChecks = 0, partyID = 0;

	struct timespec int_t_0, int_t_1;
	clock_t int_c_0, int_c_1;


	// Let's go grab some randomness.
	for(i = 0; i < checkStatSecParam; i ++)
	{
		circuitCTXs[i] = (randctx*) calloc(1, sizeof(randctx));
		circuitSeeds[i] = getIsaacContext(circuitCTXs[i]);
	}

	params = initBrainpool_256_Curve();
	groupOwn = get_128_Bit_Group(*state);


	int_t_0 = timestamp();
	int_c_0 = clock();

	outputStruct_Own = getOutputSecretsAndScheme(rawInputCircuit -> numOutputs, checkStatSecParam, *state, groupOwn);
	aList = getNaorPinkasInputs(rawInputCircuit -> numInputs_P2, checkStatSecParam, *state, params);
	NaorPinkasInputs = computeNaorPinkasInputs(cTilde, aList, rawInputCircuit -> numInputs_P2, checkStatSecParam, params);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Prep for building Own Circuits");


	int_t_0 = timestamp();
	int_c_0 = clock();

	// Build the circuits HKE
	circuitsArray_Own = buildAll_HKE_Circuits_Alt(rawInputCircuit, cTilde, NaorPinkasInputs, outputStruct_Own, params,
											circuitCTXs, checkStatSecParam, partyID);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Building Own Circuits");


	circuitsArray_Partner = (struct Circuit **) calloc(checkStatSecParam, sizeof(struct Circuit*));
	for(i = 0; i < checkStatSecParam; i ++)
	{
		circuitsArray_Partner[i] = receiveFullCircuit(writeSocket, readSocket);
	}
	for(i = 0; i < checkStatSecParam; i++)
	{
		sendCircuit(writeSocket, readSocket, circuitsArray_Own[i]);
	}

	// Each party now commits to their input values.
	commBufferLen = 0;
	commitStruct = makeCommitmentsBuilder(ctx, circuitsArray_Own, state, checkStatSecParam);
	commBuffer = serialiseC_Boxes(commitStruct, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);

	commBufferLen = 0;
	bufferOffset = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	partnersCommitStruct = deserialiseC_Boxes(commBuffer, state, &bufferOffset);


	// Send the public commitments for the Verified Secret Sharing Scheme and exchange groups.
	outputStruct_Partner = exchangePublicBoxesOfSchemes(writeSocket, readSocket, outputStruct_Own,
														rawInputCircuit -> numOutputs, checkStatSecParam);
	sendDDH_Group(writeSocket, readSocket, groupOwn);
	groupPartner = receiveDDH_Group(writeSocket, readSocket);


	int_t_0 = timestamp();
	int_c_0 = clock();

	// THIS SHOULD ONLY BE UNCOMMENTED FOR TESTING, MEANS WE CAN GET AN EASY DELTA = DELTA'
	// deltaPrime = (unsigned char *) calloc(128, sizeof(unsigned char));
	J_SetOwn = full_CnC_OT_Receiver_ECC_Alt(writeSocket, readSocket, lengthDelta,
										state, deltaPrime, &OT_Outputs, checkStatSecParam, 1024);

	OT_Inputs = getAllInputKeysSymm(circuitsArray_Own, checkStatSecParam, partyID);


	NaorPinkas_OT_Sender_Transfer(writeSocket, readSocket, circuitsArray_Own[0] -> numInputsExecutor, OT_Inputs, state, checkStatSecParam, queries_Partner, params, C);

	commBuffer = receiveBoth(readSocket, commBufferLen);
	delta = deserialiseK0sAndDelta(commBuffer, circuitsArray_Partner, rawInputCircuit -> numInputs_P1, checkStatSecParam);
	
	if(0 == memcmp(delta, deltaPrime, 128))
	{
		inputBit = 0x01;
	}

	startOfInputChain = convertArrayToChain(&inputBit, 1, rawInputCircuit -> numInputs_P1);
	for(i = 0; i < checkStatSecParam; i++)
	{
		tempWire = circuitsArray_Own[i] -> gates[rawInputCircuit -> numInputs_P1] -> outputWire;
		tempWire -> wirePermedValue = inputBit ^ (tempWire -> wirePerm & 0x01);
	}

	setDeltaXOR_onCircuitInputs(circuitsArray_Partner, OT_Outputs, inputBit, delta, deltaPrime, J_SetOwn,
								rawInputCircuit -> numInputs_P1, lengthDelta, checkStatSecParam);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "subOT - Receiver");


	// Don't need to generate own J_set, has already been done in the subOT stage.
	// J_SetOwn = generateJ_Set(stat_SecParam);
	J_SetPartner = getPartnerJ_Set(writeSocket, readSocket, J_SetOwn, stat_SecParam / 2, stat_SecParam);

	// Having exchanged J_sets the parties now reveal information needed to open the check circuits.
	commBuffer = jSetRevealSerialise(circuitsArray_Own, startOfInputChain, NaorPinkasInputs, aList, &inputBit, outputStruct_Own, circuitSeeds, J_SetPartner, rawInputCircuit -> numInputs_P2,
									rawInputCircuit -> numOutputs, checkStatSecParam, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);
	commBuffer = receiveBoth(readSocket, commBufferLen);
	partnerReveals = jSetRevealDeserialise(circuitsArray_Partner, commBuffer, J_SetOwn, rawInputCircuit -> numInputs_P1, rawInputCircuit -> numOutputs, checkStatSecParam);
	free(commBuffer);


	// Having received all that they need to verify check circuits, they now verifiy them.
	jSetChecks = HKE_Step5_Checks(writeSocket, readSocket, rawInputCircuit, circuitsArray_Partner, C, partnerReveals, NaorPinkasInputs,
								outputStruct_Partner, commitStruct, partnersCommitStruct,
								&inputBit, J_SetOwn, J_SetPartner, checkStatSecParam, groupPartner, params, 0);

	setBuildersInputsNaorPinkas(circuitsArray_Partner, rawInputCircuit, partnerReveals -> builderInputsEval,
								J_SetOwn, checkStatSecParam, partyID);


	printf("\nEvaluating Circuits ");
	for(i = 0; i < checkStatSecParam; i ++)
	{
		if(0x00 == J_SetOwn[i])
		{
			printf("%d, ", i);
			fflush(stdout);
			runCircuitExec( circuitsArray_Partner[i], writeSocket, readSocket );
		}
	}
	printf("\n");

	binaryOutput = HKE_OutputDetermination(writeSocket, readSocket, state, circuitsArray_Partner, rawInputCircuit, groupPartner,
										partnerReveals, outputStruct_Own, outputStruct_Partner, checkStatSecParam, J_SetOwn, &commBufferLen, partyID);

	printf("Candidate Output binary : ");
	for(i = 0; i < commBufferLen; i ++)
	{
		printf("%X", binaryOutput[i]);
	}
	printf("\n");


	bufferOffset = 0;
	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	logChecks |= Step5_CheckLogarithms(commBuffer, partnerReveals -> builderInputsEval, queries_Partner,
									params, J_SetOwn, checkStatSecParam, rawInputCircuit -> numInputs_P1,
									&bufferOffset);

	if(inputBit == 0)
	{
		binaryOutput = NULL;
	}

	for(i = 0; i < checkStatSecParam; i ++)
	{
		freeCircuitStruct(circuitsArray_Partner[i], 0);
	}

	return binaryOutput;
}









// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------




struct eccPoint **concatQueriesExecutor(struct eccPoint **xInputQuries, int inputSize,
										struct eccPoint **deltaQueries, int lengthDelta)
{
	struct eccPoint **outputQueries;
	int i, j = 0;

	outputQueries = (struct eccPoint **) calloc(inputSize + lengthDelta, sizeof(struct eccPoint *));

	for(i = 0; i < inputSize; i ++)
	{
		outputQueries[i] = xInputQuries[i];
	}
	for(; i < inputSize + lengthDelta; i ++)
	{
		outputQueries[i] = deltaQueries[j];
		j ++;
	}

	return outputQueries;
}




unsigned char *SC_DetectCheatingExecutor_HKE_Alt(int writeSocket, int readSocket, struct RawCircuit *rawInputCircuit,
												int length_X_Input, unsigned char *deltaPrime, int lengthDelta,
												struct eccPoint **queries_Partner, struct eccPoint *C, struct eccPoint *cTilde,
												int checkStatSecParam, gmp_randstate_t *state, randctx *ctx)
{
	struct Circuit **circuitsArray_Partner, **circuitsArray_Own;
	struct idAndValue *startOfInputChain, *curItem;
	struct jSetRevealHKE *partnerReveals;
	struct secCompExecutorOutput *returnStruct;

	struct builderInputCommitStruct *commitStruct, *partnersCommitStruct;
	struct HKE_Output_Struct_Builder *outputStruct_Own, *outputStruct_Partner;
	struct DDH_Group *groupOwn, *groupPartner;
	struct eccParams *params;
	struct wire *tempWire;

	struct OT_NP_Receiver_Query **deltaPrimeQueries;
	struct eccPoint **concated_P1_Queries, **deltaPartnerQueries;

	ub4 **circuitSeeds = (ub4 **) calloc(checkStatSecParam, sizeof(ub4*));
	randctx **circuitCTXs = (randctx **) calloc(checkStatSecParam, sizeof(randctx*));
	struct eccPoint ***NaorPinkasInputs;
	mpz_t **aList;

	unsigned char *commBuffer, *J_SetOwn, *J_SetPartner, *deltaPrimeExpanded;
	unsigned char **OT_Outputs, *binaryOutput, *delta, inputBit = 0x00, ***OT_Inputs;
	int commBufferLen = 0, i, J_setSize = 0, arrayLen = 0, circuitsChecked = 0, bufferOffset = 0;
	int jSetChecks = 0, logChecks = 0, partyID = 0;

	struct timespec int_t_0, int_t_1;
	clock_t int_c_0, int_c_1;


	// Let's go grab some randomness.
	for(i = 0; i < checkStatSecParam; i ++)
	{
		circuitCTXs[i] = (randctx*) calloc(1, sizeof(randctx));
		circuitSeeds[i] = getIsaacContext(circuitCTXs[i]);
	}

	params = initBrainpool_256_Curve();
	groupOwn = get_128_Bit_Group(*state);


	int_t_0 = timestamp();
	int_c_0 = clock();

	outputStruct_Own = getOutputSecretsAndScheme(rawInputCircuit -> numOutputs, checkStatSecParam, *state, groupOwn);
	aList = getNaorPinkasInputs(rawInputCircuit -> numInputs_P2, checkStatSecParam, *state, params);
	NaorPinkasInputs = computeNaorPinkasInputs(cTilde, aList, rawInputCircuit -> numInputs_P2, checkStatSecParam, params);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Prep for building Own Circuits");


	int_t_0 = timestamp();
	int_c_0 = clock();


	startOfInputChain = convertArrayToChain(deltaPrime, lengthDelta, rawInputCircuit -> numInputs_P1);
	// Build the circuits HKE
	circuitsArray_Own = buildAll_HKE_Circuits_Alt(rawInputCircuit, cTilde, NaorPinkasInputs, outputStruct_Own, params,
											circuitCTXs, checkStatSecParam, partyID);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Building Own Circuits");


	circuitsArray_Partner = (struct Circuit **) calloc(checkStatSecParam, sizeof(struct Circuit*));
	for(i = 0; i < checkStatSecParam; i ++)
	{
		circuitsArray_Partner[i] = receiveFullCircuit(writeSocket, readSocket);
		setCircuitsInputs_Values(startOfInputChain, circuitsArray_Partner[i], 0xFF);
	}
	for(i = 0; i < checkStatSecParam; i++)
	{
		sendCircuit(writeSocket, readSocket, circuitsArray_Own[i]);
	}

	/*
	printf("@@@\n");
	fflush(stdout);

	for(i = 0; i < circuitsArray_Own[0] -> numInputs; i ++)
	{
		circuitsArray_Own[0] -> gates[i] -> outputWire -> wireOutputKey = (unsigned char *) calloc(16, sizeof(unsigned char));
		memcpy(circuitsArray_Own[0] -> gates[i] -> outputWire -> wireOutputKey, circuitsArray_Own[0] -> gates[i] -> outputWire -> outputGarbleKeys -> key0, 16);
		circuitsArray_Own[0] -> gates[i] -> outputWire -> wirePermedValue = 0x01 & circuitsArray_Own[0] -> gates[i] -> outputWire -> wirePerm;
	}

	runCircuitExec( circuitsArray_Own[0], writeSocket, readSocket );
	*/



	// Each party now commits to their input values.
	commBufferLen = 0;
	commitStruct = makeCommitmentsBuilder(ctx, circuitsArray_Own, state, checkStatSecParam);
	commBuffer = serialiseC_Boxes(commitStruct, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);

	commBufferLen = 0;
	bufferOffset = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	partnersCommitStruct = deserialiseC_Boxes(commBuffer, state, &bufferOffset);


	// Send the public commitments for the Verified Secret Sharing Scheme and exchange groups.
	outputStruct_Partner = exchangePublicBoxesOfSchemes(writeSocket, readSocket, outputStruct_Own,
														rawInputCircuit -> numOutputs, checkStatSecParam);
	sendDDH_Group(writeSocket, readSocket, groupOwn);
	groupPartner = receiveDDH_Group(writeSocket, readSocket);


	int_t_0 = timestamp();
	int_c_0 = clock();

	// THIS SHOULD ONLY BE UNCOMMENTED FOR TESTING, MEANS WE CAN GET AN EASY DELTA = DELTA'
	// deltaPrime = (unsigned char *) calloc(128, sizeof(unsigned char));
	OT_Inputs = getAllInputKeysSymm(circuitsArray_Own, checkStatSecParam, partyID);


	deltaPrimeQueries = NaorPinkas_OT_Produce_Queries(lengthDelta, deltaPrime, state, params, cTilde);
	deltaPartnerQueries = NaorPinkas_OT_Exchange_Queries(writeSocket, readSocket, lengthDelta, lengthDelta, deltaPrimeQueries);


	concated_P1_Queries = concatQueriesExecutor(queries_Partner, length_X_Input,
											deltaPartnerQueries, lengthDelta);

	NaorPinkas_OT_Sender_Transfer(writeSocket, readSocket, length_X_Input + lengthDelta, OT_Inputs,
								state, checkStatSecParam, concated_P1_Queries, params, C);
	OT_Outputs = NaorPinkas_OT_Receiver_Transfer(writeSocket, readSocket, lengthDelta,
												deltaPrime, state, checkStatSecParam, deltaPrimeQueries, params, cTilde);

	setInputsFromNaorPinkas(circuitsArray_Partner, OT_Outputs, checkStatSecParam, partyID);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "subOT - Receiver");


	J_SetOwn = generateJ_Set(checkStatSecParam);
	J_SetPartner = getPartnerJ_Set(writeSocket, readSocket, J_SetOwn, checkStatSecParam / 2, checkStatSecParam);


	// Having exchanged J_sets the parties now reveal information needed to open the check circuits.
	commBuffer = jSetRevealSerialise(circuitsArray_Own, startOfInputChain, NaorPinkasInputs, aList, deltaPrime,
									outputStruct_Own, circuitSeeds, J_SetPartner, rawInputCircuit -> numInputs_P2,
									rawInputCircuit -> numOutputs, checkStatSecParam, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);

	commBuffer = receiveBoth(readSocket, commBufferLen);
	partnerReveals = jSetRevealDeserialise(circuitsArray_Partner, commBuffer, J_SetOwn, rawInputCircuit -> numInputs_P1,
										rawInputCircuit -> numOutputs, checkStatSecParam);
	free(commBuffer);


	// Having received all that they need to verify check circuits, they now verifiy them.
	jSetChecks = HKE_Step5_Checks(writeSocket, readSocket, rawInputCircuit, circuitsArray_Partner, C, partnerReveals, NaorPinkasInputs,
								outputStruct_Partner, commitStruct, partnersCommitStruct,
								deltaPrime, J_SetOwn, J_SetPartner, checkStatSecParam, groupPartner, params, 0);

	setBuildersInputsNaorPinkas(circuitsArray_Partner, rawInputCircuit, partnerReveals -> builderInputsEval,
								J_SetOwn, checkStatSecParam, partyID);

	int j, h;
	for(i = 0; i < length_X_Input + lengthDelta * 2; i ++)
	{
		for(j = 0; j < checkStatSecParam; j ++)
		{
			if(0x00 == J_SetOwn[j])
			{
				printf("(%d, %d)  %d  = ", i, j,  circuitsArray_Partner[j] -> gates[i] -> outputWire -> wirePermedValue);
				for(h = 0; h < 16; h ++)
				{
					printf("%02X", circuitsArray_Partner[j] -> gates[i] -> outputWire -> wireOutputKey[h]);
				}
				printf("\n");
			}
		}
	}


	printf("\nEvaluating Circuits ");
	for(i = 0; i < checkStatSecParam; i ++)
	{
		if(0x00 == J_SetOwn[i])
		{
			printf("%d, ", i);
			fflush(stdout);
			runCircuitExec( circuitsArray_Partner[i], writeSocket, readSocket );
		}
	}
	printf("\n");

	/*
	binaryOutput = HKE_OutputDetermination(writeSocket, readSocket, state, circuitsArray_Partner, rawInputCircuit, groupPartner,
										partnerReveals, outputStruct_Own, outputStruct_Partner, checkStatSecParam, J_SetOwn, &commBufferLen, partyID);

	printf("Candidate Output binary : ");
	for(i = 0; i < commBufferLen; i ++)
	{
		printf("%X", binaryOutput[i]);
	}
	printf("\n");


	bufferOffset = 0;
	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	logChecks |= Step5_CheckLogarithms(commBuffer, partnerReveals -> builderInputsEval, queries_Partner,
									params, J_SetOwn, checkStatSecParam, rawInputCircuit -> numInputs_P1,
									&bufferOffset);
	*/

	if(inputBit == 0)
	{
		binaryOutput = NULL;
	}

	for(i = 0; i < checkStatSecParam; i ++)
	{
		freeCircuitStruct(circuitsArray_Partner[i], 0);
	}

	return binaryOutput;
}

