unsigned char *concatInputStrings(unsigned char *input1, int length1,
								unsigned char *input2, int length2)
{
	unsigned char *concatOutput = (unsigned char *) calloc(length1 + length2, sizeof(unsigned char));
	int i;

	memcpy(concatOutput, input1, length1);
	memcpy(concatOutput + length1, input2, length2);

	return concatOutput;
}


void concatInputChains(struct idAndValue *chain1, struct idAndValue *chain2)
{
	struct idAndValue *temp = chain1 -> next;

	while(temp -> next != NULL)
	{
		temp = temp -> next;
	}

	temp -> next = chain2;
}


struct OT_NP_Receiver_Query **concatQueriesBuilder(struct OT_NP_Receiver_Query **xInputQuries, int inputSize,
												struct OT_NP_Receiver_Query **deltaQueries, int lengthDelta)
{
	struct OT_NP_Receiver_Query **outputQueries;
	int i, j;

	outputQueries = (struct OT_NP_Receiver_Query **) calloc(inputSize + lengthDelta, sizeof(struct OT_NP_Receiver_Query *));

	for(i = 0; i < inputSize; i ++)
	{
		outputQueries[i] = xInputQuries[i];
	}
	j = 0;
	for(; i < inputSize + lengthDelta; i ++)
	{
		outputQueries[i] = deltaQueries[j];
		j ++;
	}

	return outputQueries;
}



void SC_DetectCheatingBuilder_HKE(int writeSocket, int readSocket, struct RawCircuit *rawInputCircuit,
								struct idAndValue *startOfInputChain, int length_X_Input, unsigned char *delta, int lengthDelta,
								struct OT_NP_Receiver_Query **queries_Own, struct eccPoint *C,
								struct eccPoint *cTilde, int checkStatSecParam, gmp_randstate_t *state, randctx *ctx)
{
	struct Circuit **circuitsArray_Own, **circuitsArray_Partner;
	struct eccParams *params;

	struct HKE_Output_Struct_Builder *outputStruct_Own, *outputStruct_Partner;
	struct builderInputCommitStruct *commitStruct, *partnersCommitStruct;
	struct DDH_Group *groupOwn, *groupPartner;
	struct jSetRevealHKE *partnerReveals;
	struct eccPoint ***NP_consistentInputs;
	struct idAndValue *curItem;
	mpz_t **aList;

	struct OT_NP_Receiver_Query **deltaQueries, **concated_P1_Queries;
	struct eccPoint **queries_Partner;

	unsigned char *commBuffer, *J_SetOwn, *J_SetPartner, ***OT_Inputs, **OT_Outputs;
	unsigned char *deltaExpanded, *inputBitsOwn, *binaryOutput, *concatInput;
	int commBufferLen = 0, i, j, k, J_setSize = 0, arrayLen = 0, bufferOffset = 0;
	int jSetChecks = 0, logChecks = 0, partyID = 1;

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
	groupOwn = get_128_Bit_Group(*state);

	curItem = startOfInputChain;
	inputBitsOwn = convertChainIntoArray(curItem, length_X_Input);


	aList = getNaorPinkasInputs(rawInputCircuit -> numInputs_P1, checkStatSecParam, *state, params);
	NP_consistentInputs = computeNaorPinkasInputs(cTilde, aList, rawInputCircuit -> numInputs_P1, checkStatSecParam, params);


	outputStruct_Own = getOutputSecretsAndScheme(rawInputCircuit -> numOutputs, checkStatSecParam, *state, groupOwn);
	circuitsArray_Own = buildAll_HKE_Circuits_Alt(rawInputCircuit, cTilde, NP_consistentInputs, outputStruct_Own, params,
											circuitCTXs, checkStatSecParam, partyID);

	deltaExpanded = expandBitString(delta, lengthDelta);
	concatInput = concatInputStrings(inputBitsOwn, length_X_Input,
								deltaExpanded, lengthDelta);
	startOfInputChain = convertArrayToChain(concatInput, length_X_Input + lengthDelta, 0);

	for(i = 0; i < checkStatSecParam; i++)
	{
		sendCircuit(writeSocket, readSocket, circuitsArray_Own[i]);
	}
	circuitsArray_Partner = (struct Circuit **) calloc(checkStatSecParam, sizeof(struct Circuit*));
	for(i = 0; i < checkStatSecParam; i ++)
	{
		circuitsArray_Partner[i] = receiveFullCircuit(writeSocket, readSocket);
		setCircuitsInputs_Values(startOfInputChain, circuitsArray_Partner[i], 0xFF);
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


	// THIS SHOULD ONLY BE UNCOMMENTED FOR TESTING, MEANS WE CAN GET AN EASY DELTA = DELTA'
	// delta = (unsigned char *) calloc(16, sizeof(unsigned char));
	// OT_Inputs = getCheckCircuitOT_Inputs(circuitsArray_Own, delta, checkStatSecParam, lengthDelta);

	OT_Inputs = getAllInputKeysSymm(circuitsArray_Own, checkStatSecParam, partyID);
	

	int_t_0 = timestamp();
	int_c_0 = clock();

	deltaQueries = NaorPinkas_OT_Produce_Queries(lengthDelta, deltaExpanded, state, params, cTilde);
	queries_Partner = NaorPinkas_OT_Exchange_Queries(writeSocket, readSocket, lengthDelta, lengthDelta, deltaQueries);


	concated_P1_Queries = concatQueriesBuilder(queries_Own, length_X_Input,
											deltaQueries, lengthDelta);

	NaorPinkas_OT_Sender_Transfer(writeSocket, readSocket, circuitsArray_Own[0] -> numInputsExecutor, OT_Inputs,
								state, checkStatSecParam, queries_Partner, params, C);
	OT_Outputs = NaorPinkas_OT_Receiver_Transfer(writeSocket, readSocket, circuitsArray_Partner[0] -> numInputsExecutor,
												concatInput, state, checkStatSecParam, concated_P1_Queries, params, cTilde);


	setInputsFromNaorPinkas(circuitsArray_Partner, OT_Outputs, checkStatSecParam, partyID);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "subOT - Sender");

	J_SetOwn = generateJ_Set(checkStatSecParam);
	J_SetPartner = getPartnerJ_Set(writeSocket, readSocket, J_SetOwn, checkStatSecParam / 2, checkStatSecParam);


	commBuffer = jSetRevealSerialise(circuitsArray_Own, startOfInputChain, NP_consistentInputs, aList,
									concatInput, outputStruct_Own, circuitSeeds, J_SetPartner,
									rawInputCircuit -> numInputs_P1, rawInputCircuit -> numOutputs, checkStatSecParam, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);

	commBuffer = receiveBoth(readSocket, commBufferLen);
	partnerReveals = jSetRevealDeserialise(circuitsArray_Partner, commBuffer, J_SetOwn, rawInputCircuit -> numInputs_P2,
										rawInputCircuit -> numOutputs, checkStatSecParam);
	free(commBuffer);


	jSetChecks = HKE_Step5_Checks(writeSocket, readSocket, rawInputCircuit, circuitsArray_Partner, C,
								partnerReveals, NP_consistentInputs, outputStruct_Partner,
								commitStruct, partnersCommitStruct,
								concatInput, J_SetOwn, J_SetPartner, checkStatSecParam, groupPartner, params, 1);


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


	HKE_OutputDetermination(writeSocket, readSocket, state, circuitsArray_Partner, rawInputCircuit, groupPartner,
							partnerReveals, outputStruct_Own, outputStruct_Partner, checkStatSecParam,
							J_SetOwn, &commBufferLen, partyID);


	commBuffer = Step5_CalculateLogarithms(NP_consistentInputs, aList, concated_P1_Queries, params, concatInput,
										J_SetPartner, checkStatSecParam, rawInputCircuit -> numInputs_P1, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);

	bufferOffset = 0;
	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);

	logChecks |= Step5_CheckLogarithms(commBuffer, partnerReveals -> builderInputsEval, queries_Partner,
									params, J_SetOwn, checkStatSecParam, rawInputCircuit -> numInputs_P2,
									&bufferOffset);
}


