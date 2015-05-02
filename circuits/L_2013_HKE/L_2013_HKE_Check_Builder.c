unsigned char *serialiseToDeltaKeys(struct eccPoint ***NaorPinkasInputs, mpz_t **aList, unsigned char *inputBits,
									int lengthX, int lengthDelta, int numCircuits, int *outputLength)
{
	unsigned char *outputBuffer;
	int i, j, tempOffset = 0;
	int aListLen = 0;


	(*outputLength) = 0;

	for(j = 0; j < numCircuits; j ++)
	{
		for(i = lengthX; i < lengthX + lengthDelta; i ++)
		{
			if(0x00 == inputBits[i])
			{
				(*outputLength) += sizeOfSerial_ECCPoint(NaorPinkasInputs[j][2*i]);
			}
			else
			{
				(*outputLength) += sizeOfSerial_ECCPoint(NaorPinkasInputs[j][2*i + 1]);
			}
		}
	}

	outputBuffer = (unsigned char *) calloc(*outputLength, sizeof(unsigned char));

	for(j = 0; j < numCircuits; j ++)
	{
		for(i = lengthX; i < lengthX + lengthDelta; i ++)
		{
			if(0x00 == inputBits[i])
			{
				serialise_ECC_Point(NaorPinkasInputs[j][2*i], outputBuffer, &tempOffset);
			}
			else
			{
				serialise_ECC_Point(NaorPinkasInputs[j][2*i + 1], outputBuffer, &tempOffset);
			}
		}
	}

	return outputBuffer;
}




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



void SC_DetectCheatingBuilder_HKE_Alt(int writeSocket, int readSocket, struct RawCircuit *rawInputCircuit,
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
	struct eccPoint ***NP_consistentInputs, ***NP_Inputs_Partner;
	struct idAndValue *curItem;
	mpz_t **aList;

	struct OT_NP_Receiver_Query **deltaQueries, **concated_P1_Queries;
	struct eccPoint **queries_Partner;

	unsigned char *commBuffer, *J_SetOwn, *J_SetPartner, **J_setPair, ***OT_Inputs, **OT_Outputs;
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

	// Zero the sub-computation bytes sent/received counters.
	zeroBothSubCounters();

	params = initBrainpool_256_Curve();
	groupOwn = get_128_Bit_Group(*state);

	curItem = startOfInputChain;
	inputBitsOwn = convertChainIntoArray(curItem, length_X_Input);

	int_t_0 = timestamp();
	int_c_0 = clock();

	aList = getNaorPinkasInputs(rawInputCircuit -> numInputs_P1, checkStatSecParam, *state, params);
	NP_consistentInputs = computeNaorPinkasInputs(cTilde, aList, rawInputCircuit -> numInputs_P1, checkStatSecParam, params);

	outputStruct_Own = getOutputSecretsAndScheme(rawInputCircuit -> numOutputs, checkStatSecParam, *state, groupOwn);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Circuit building prep");
	printZeroBothSubCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	circuitsArray_Own = buildAll_HKE_Circuits_Alt(rawInputCircuit, NP_consistentInputs, outputStruct_Own, params,
											circuitCTXs, checkStatSecParam, partyID);

	deltaExpanded = expandBitString(delta, lengthDelta);
	concatInput = concatInputStrings(inputBitsOwn, length_X_Input,
								deltaExpanded, lengthDelta);
	startOfInputChain = convertArrayToChain(concatInput, length_X_Input + lengthDelta, 0);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Circuits built.");
	printZeroBothSubCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

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


	commBuffer = serialiseToDeltaKeys(NP_consistentInputs, aList, concatInput, length_X_Input, lengthDelta, checkStatSecParam, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Circuit and sundry sent.");
	printZeroBothSubCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

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

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Make and exchange commitments");
	printZeroBothSubCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	// Send the public commitments for the Verified Secret Sharing Scheme and exchange groups.
	outputStruct_Partner = exchangePublicBoxesOfSchemes(writeSocket, readSocket, outputStruct_Own,
													rawInputCircuit -> numOutputs, checkStatSecParam);
	sendDDH_Group(writeSocket, readSocket, groupOwn);
	groupPartner = receiveDDH_Group(writeSocket, readSocket);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Exchanged Secret Sharing Schemes");
	printZeroBothSubCounters();


	// THIS SHOULD ONLY BE UNCOMMENTED FOR TESTING, MEANS WE CAN GET AN EASY DELTA = DELTA'
	// delta = (unsigned char *) calloc(16, sizeof(unsigned char));
	OT_Inputs = getAllInputKeysSymm(circuitsArray_Own, checkStatSecParam, partyID);
	

	int_t_0 = timestamp();
	int_c_0 = clock();

	deltaQueries = NaorPinkas_OT_Produce_Queries(lengthDelta, deltaExpanded, state, params, cTilde);
	queries_Partner = NaorPinkas_OT_Exchange_Queries(writeSocket, readSocket, lengthDelta,
													rawInputCircuit -> numInputs_P2, deltaQueries);


	concated_P1_Queries = concatQueriesBuilder(queries_Own, length_X_Input,
											deltaQueries, lengthDelta);

	NaorPinkas_OT_Sender_Transfer(writeSocket, readSocket, rawInputCircuit -> numInputs_P2, OT_Inputs,
								state, checkStatSecParam, queries_Partner, params, C);
	OT_Outputs = NaorPinkas_OT_Receiver_Transfer(writeSocket, readSocket, rawInputCircuit -> numInputs_P1,
												concatInput, state, checkStatSecParam, concated_P1_Queries, params, cTilde);

	sendBoth(writeSocket, deltaExpanded, lengthDelta);

	setInputsFromNaorPinkas(circuitsArray_Partner, OT_Outputs, checkStatSecParam, partyID);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "subOT - Sender");
	printZeroBothSubCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	randctx *tempCTX = getRandCtxByCoinToss(writeSocket, readSocket, ctx, state, partyID);
	J_setPair = getJ_Set(tempCTX, partyID, checkStatSecParam);
	J_SetOwn = J_setPair[0];
	J_SetPartner = J_setPair[1];

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Coin flip.");
	printZeroBothSubCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	commBuffer = jSetRevealSerialise(circuitsArray_Own, startOfInputChain, NP_consistentInputs, aList,
									concatInput, outputStruct_Own, circuitSeeds, J_SetPartner,
									rawInputCircuit -> numInputs_P1, rawInputCircuit -> numOutputs,
									checkStatSecParam, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);

	commBuffer = receiveBoth(readSocket, commBufferLen);
	partnerReveals = jSetRevealDeserialise(circuitsArray_Partner, commBuffer, J_SetOwn, rawInputCircuit -> numInputs_P2,
										rawInputCircuit -> numOutputs, checkStatSecParam);
	free(commBuffer);


	commBuffer = serialiseSeeds(circuitSeeds, J_SetPartner, checkStatSecParam, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);

	commBuffer = receiveBoth(readSocket, commBufferLen);
	partnerReveals -> revealedSeeds = deserialiseSeeds(commBuffer, J_SetOwn, checkStatSecParam);
	free(commBuffer);



	NP_Inputs_Partner = computeNaorPinkasInputsForJSet(C, partnerReveals -> aListRevealed, circuitsArray_Partner[0] -> numInputsBuilder,
													checkStatSecParam, params, J_SetOwn);

	jSetChecks = HKE_Step5_Checks(writeSocket, readSocket, rawInputCircuit, circuitsArray_Partner, C,
								partnerReveals, NP_consistentInputs, NP_Inputs_Partner, outputStruct_Partner,
								commitStruct, partnersCommitStruct, concatInput,
								J_SetOwn, J_SetPartner, checkStatSecParam, groupPartner, params, 1);


	setBuildersInputsNaorPinkas(circuitsArray_Partner, rawInputCircuit, partnerReveals -> builderInputsEval,
								J_SetOwn, checkStatSecParam, partyID);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Initial J-set checks.");
	printZeroBothSubCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

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

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Circuit Evaluation");
	printZeroBothSubCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

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

	printf("Sub-Log Checks: %d\n", logChecks);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Sub-Log Checks");
	printZeroBothSubCounters();


	// You can uncomment this line to see what the Builder sees output from the sub-computation.
	// However, it'll be garbage so not very interesting except for checking it's working.
	// printMajorityOutputAsBinary(circuitsArray_Partner, checkStatSecParam, J_SetOwn);

	int_t_0 = timestamp();
	int_c_0 = clock();

	HKE_OutputDetermination(writeSocket, readSocket, state, circuitsArray_Partner, rawInputCircuit, groupPartner,
							partnerReveals, outputStruct_Own, outputStruct_Partner, checkStatSecParam,
							J_SetOwn, &commBufferLen, partyID);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Output determination");
	printZeroBothSubCounters();
}


