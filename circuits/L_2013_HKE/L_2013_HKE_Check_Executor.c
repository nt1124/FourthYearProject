struct eccPoint ***deserialiseToDeltaKeys(unsigned char *inputBuffer,
										int lengthDelta, int numCircuits, int *bufferOffset)
{
	struct eccPoint ***outputPoints;
	int i, j, k, tempOffset = *bufferOffset;
	int aListLen = 0, sharesLen = 0, seedLengths = (numCircuits / 2) * 256 * sizeof(ub4);


	outputPoints = (struct eccPoint ***) calloc(numCircuits, sizeof(struct eccPoint **));

	for(j = 0; j < numCircuits; j ++)
	{
		outputPoints[j] = (struct eccPoint **) calloc(lengthDelta, sizeof(struct eccPoint *));

		for(i = 0; i < lengthDelta; i ++)
		{
			outputPoints[j][i] = deserialise_ECC_Point(inputBuffer, &tempOffset);
		}
	}

	*bufferOffset = tempOffset;

	return outputPoints;
}



unsigned char testDeltaKeys(struct eccPoint ***commitedDeltaKeys, struct eccPoint ***checkInputs, unsigned char *J_SetOwn,
							unsigned char *delta, int length_X_Input, int lengthDelta, int numCircuits)
{
	unsigned char testOutput = 0;
	int i, j, k;


	for(j = 0; j < numCircuits; j ++)
	{
		if(0x01 == J_SetOwn[j])
		{
			for(i = 0; i < lengthDelta; i ++)
			{
				k = (2 * length_X_Input) + (2 * i) + delta[i];
				testOutput |= eccPointsEqual(commitedDeltaKeys[j][i], checkInputs[j][k]);
			}
		}
	}


	return testOutput;
}



unsigned char *generateXOR_Key_Bits(randctx *ctx, int keyLength, unsigned char *deltaPrimeBits, int deltaLength)
{
	unsigned char *outputBits;
	int i, j = 0;


	outputBits = generateIsaacRandBytes(ctx, keyLength, keyLength + deltaLength);

	for(i = 0; i < keyLength; i ++)
	{
		outputBits[i] &= 0x01; 
	}
	for(; i < keyLength + deltaLength; i ++)
	{
		outputBits[i] = deltaPrimeBits[j];
		j ++; 
	}


	return outputBits;
}


struct eccPoint **concatQueriesExecutor(struct eccPoint **xInputQuries, int inputSize,
										struct eccPoint **deltaQueries, int lengthDelta)
{
	struct eccPoint **outputQueries;
	int i, j;

	outputQueries = (struct eccPoint **) calloc(inputSize + lengthDelta, sizeof(struct eccPoint *));

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
	struct eccPoint **concated_P1_Queries, **deltaPartnerQueries, ***deltaCommits, ***NP_Inputs_Partner;

	ub4 **circuitSeeds = (ub4 **) calloc(checkStatSecParam, sizeof(ub4*));
	randctx **circuitCTXs = (randctx **) calloc(checkStatSecParam, sizeof(randctx*)), *tempCTX;
	struct eccPoint ***NP_consistentInputs;
	mpz_t **aList;

	unsigned char *commBuffer, *J_SetOwn, *J_SetPartner, *deltaPrimeExpanded, **J_setPair;
	unsigned char **OT_Outputs, *binaryOutput, *deltaFromPartner, equalityBit = 0x00, ***OT_Inputs;
	int commBufferLen = 0, i, j, k, J_setSize = 0, arrayLen = 0, circuitsChecked = 0, bufferOffset = 0;
	int jSetChecks = 0, logChecks = 0, partyID = 0;

	struct timespec int_t_0, int_t_1;
	clock_t int_c_0, int_c_1;


	// Let's go grab some randomness.
	for(i = 0; i < checkStatSecParam; i ++)
	{
		circuitCTXs[i] = (randctx*) calloc(1, sizeof(randctx));
		circuitSeeds[i] = getIsaacContext(circuitCTXs[i]);
	}

	// Zero the sub-computation bytes sent/received counters.
	zeroBothSubCounters();

	params = initBrainpool_256_Curve();
	groupOwn = get_128_Bit_Group(*state);


	int_t_0 = timestamp();
	int_c_0 = clock();

	outputStruct_Own = getOutputSecretsAndScheme(rawInputCircuit -> numOutputs, checkStatSecParam, *state, groupOwn);
	aList = getNaorPinkasInputs(rawInputCircuit -> numInputs_P2, checkStatSecParam, *state, params);
	NP_consistentInputs = computeNaorPinkasInputs(cTilde, aList, rawInputCircuit -> numInputs_P2, checkStatSecParam, params);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Prep for building Own Circuits");
	printZeroBothSubCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();


	unsigned char *keyPlusDeltaPrime = generateXOR_Key_Bits(ctx, length_X_Input, deltaPrime, lengthDelta);
	startOfInputChain = convertArrayToChain(keyPlusDeltaPrime, length_X_Input + lengthDelta,
											rawInputCircuit -> numInputs_P1);
	// startOfInputChain = convertArrayToChain(deltaPrime, lengthDelta, rawInputCircuit -> numInputs_P1);
	// Build the circuits HKE
	circuitsArray_Own = buildAll_HKE_Circuits_Alt(rawInputCircuit, NP_consistentInputs, outputStruct_Own, params,
											circuitCTXs, checkStatSecParam, partyID);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Building Own Circuits");
	printZeroBothSubCounters();



	int_t_0 = timestamp();
	int_c_0 = clock();

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

	commBufferLen = 0;
	bufferOffset = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	deltaCommits = deserialiseToDeltaKeys(commBuffer, lengthDelta, checkStatSecParam, &bufferOffset);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Exchanging Circuits + delta commit.");
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
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Make commitments.");
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
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Exchange Secret Sharing Schemes.");
	printZeroBothSubCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	// THIS SHOULD ONLY BE UNCOMMENTED FOR TESTING, MEANS WE CAN GET AN EASY DELTA = DELTA'
	// deltaPrime = (unsigned char *) calloc(128, sizeof(unsigned char));
	OT_Inputs = getAllInputKeysSymm(circuitsArray_Own, checkStatSecParam, partyID);

	deltaPrimeQueries = NaorPinkas_OT_Produce_Queries(rawInputCircuit -> numInputs_P2, keyPlusDeltaPrime,
													state, params, cTilde);
	deltaPartnerQueries = NaorPinkas_OT_Exchange_Queries(writeSocket, readSocket, rawInputCircuit -> numInputs_P2, 
														lengthDelta, deltaPrimeQueries);

	concated_P1_Queries = concatQueriesExecutor(queries_Partner, length_X_Input,
											deltaPartnerQueries, lengthDelta);

	NaorPinkas_OT_Sender_Transfer(writeSocket, readSocket, rawInputCircuit -> numInputs_P1, OT_Inputs,
								state, checkStatSecParam, concated_P1_Queries, params, C);
	OT_Outputs = NaorPinkas_OT_Receiver_Transfer(writeSocket, readSocket, rawInputCircuit -> numInputs_P2,
												keyPlusDeltaPrime, state, checkStatSecParam, deltaPrimeQueries, params, cTilde);


	deltaFromPartner = receiveBoth(readSocket, commBufferLen);
	if(0 == memcmp(deltaFromPartner, deltaPrime, lengthDelta))
	{
		equalityBit = 0x01;
	}

	setInputsFromNaorPinkas(circuitsArray_Partner, OT_Outputs, checkStatSecParam, partyID);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "subOT - Receiver");
	printZeroBothSubCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	tempCTX = getRandCtxByCoinToss(writeSocket, readSocket, ctx, state, partyID);
	J_setPair = getJ_Set(tempCTX, partyID, checkStatSecParam);
	J_SetOwn = J_setPair[0];
	J_SetPartner = J_setPair[1];

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Coin flips");
	printZeroBothSubCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	// Having exchanged J_sets the parties now reveal information needed to open the check circuits.
	commBuffer = jSetRevealSerialise(circuitsArray_Own, startOfInputChain, NP_consistentInputs, aList,
									keyPlusDeltaPrime, outputStruct_Own, circuitSeeds, J_SetPartner,
									rawInputCircuit -> numInputs_P2, rawInputCircuit -> numOutputs,
									checkStatSecParam, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);

	commBuffer = receiveBoth(readSocket, commBufferLen);
	partnerReveals = jSetRevealDeserialise(circuitsArray_Partner, commBuffer, J_SetOwn, rawInputCircuit -> numInputs_P1,
										rawInputCircuit -> numOutputs, checkStatSecParam);
	free(commBuffer);


	commBuffer = serialiseSeeds(circuitSeeds, J_SetPartner, checkStatSecParam, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);

	commBuffer = receiveBoth(readSocket, commBufferLen);
	partnerReveals -> revealedSeeds = deserialiseSeeds(commBuffer, J_SetOwn, checkStatSecParam);
	free(commBuffer);



	// Having received all that they need to verify check circuits, they now verifiy them.
	NP_Inputs_Partner = computeNaorPinkasInputsForJSet(C, partnerReveals -> aListRevealed, circuitsArray_Partner[0] -> numInputsBuilder,
													checkStatSecParam, params, J_SetOwn);
	
	printf("Delta Keys commitment : %d\n", testDeltaKeys(deltaCommits, NP_Inputs_Partner, J_SetOwn, deltaFromPartner,
														length_X_Input, lengthDelta, checkStatSecParam));

	jSetChecks = HKE_Step5_Checks(writeSocket, readSocket, rawInputCircuit, circuitsArray_Partner, C, partnerReveals,
								NP_consistentInputs, NP_Inputs_Partner, outputStruct_Partner, commitStruct,
								partnersCommitStruct, keyPlusDeltaPrime, J_SetOwn, J_SetPartner, checkStatSecParam,
								groupPartner, params, 0);

	setBuildersInputsNaorPinkas(circuitsArray_Partner, rawInputCircuit, partnerReveals -> builderInputsEval,
								J_SetOwn, checkStatSecParam, partyID);
	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Initial J-Set checks");
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
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Evaluate Circuits");
	printZeroBothSubCounters();


	// Check the Builder's inputs to the evaluation circuits is correct.
	int_t_0 = timestamp();
	int_c_0 = clock();

	bufferOffset = 0;
	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	logChecks |= Step5_CheckLogarithms(commBuffer, partnerReveals -> builderInputsEval, concated_P1_Queries,
									params, J_SetOwn, checkStatSecParam, rawInputCircuit -> numInputs_P1,
									&bufferOffset);

	commBuffer = Step5_CalculateLogarithms(NP_consistentInputs, aList, deltaPrimeQueries, params, keyPlusDeltaPrime,
										J_SetPartner, checkStatSecParam, rawInputCircuit -> numInputs_P2, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);
	printf("Sub-Log Checks: %d\n", logChecks);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Sub-Log checks");
	printZeroBothSubCounters();


	// 
	int_t_0 = timestamp();
	int_c_0 = clock();

	binaryOutput = HKE_OutputDetermination(writeSocket, readSocket, state, circuitsArray_Partner, rawInputCircuit, groupPartner,
										partnerReveals, outputStruct_Own, outputStruct_Partner, checkStatSecParam, J_SetOwn, &commBufferLen, partyID);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Output Determination");
	printZeroBothSubCounters();


	// We now XOR the output with the key we provided, giving us the true output.
	printf("Sub-Computation Output binary : ");
	for(i = 0; i < rawInputCircuit -> numOutputs; i ++)
	{
		binaryOutput[i] ^= keyPlusDeltaPrime[i];
		printf("%X", binaryOutput[i]);
	}
	printf("\n");


	// Check, did we enter a correct deltaPrime? If not we know the output is worthless.
	if(equalityBit == 0)
	{
		binaryOutput = NULL;
	}

	/*
	// Tidy up after ourselves.
	for(i = 0; i < checkStatSecParam; i ++)
	{
		freeCircuitStruct(circuitsArray_Partner[i], 0);
	}
	*/

	return binaryOutput;
}

