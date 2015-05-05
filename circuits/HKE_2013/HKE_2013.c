struct Circuit **buildAll_HKE_Circuits_Alt(struct RawCircuit *rawInputCircuit, struct eccPoint ***NaorPinkasInputs,
										struct HKE_Output_Struct_Builder *outputStruct_Own, struct eccParams *params,
										randctx **circuitCTXs,
										int numCircuits, int partyID)
{
	struct Circuit **circuitsArray_Own = (struct Circuit **) calloc(numCircuits, sizeof(struct Circuit*));
	struct idAndValue *start;
	mpz_t *outputKeysLocals;
	int i, j;


	#pragma omp parallel for private(i, j, outputKeysLocals) schedule(auto)
	for(j = 0; j < numCircuits; j++)
	{
		outputKeysLocals = getOutputKeys(outputStruct_Own, rawInputCircuit -> numOutputs, j);
		circuitsArray_Own[j] = readInCircuit_FromRaw_HKE_2013(circuitCTXs[j], rawInputCircuit, NaorPinkasInputs[j], outputKeysLocals, params, partyID);

		for(i = 0; i < 2 * rawInputCircuit -> numOutputs; i ++)
		{
			mpz_clear(outputKeysLocals[i]);
		}
		free(outputKeysLocals);
	}


	return circuitsArray_Own;
}



int HKE_performCircuitChecks(struct Circuit **circuitsArrayPartner, struct RawCircuit *rawInputCircuit,
							struct eccPoint ***NaorPinkasInputs, struct jSetRevealHKE *revealStruct, struct eccParams *params,
							unsigned char *J_set, int J_setSize, int numCircuits, int partyID_Own)
{
	struct Circuit *tempGarbleCircuit;
	int i, j, k = 0, *idList = (int*) calloc(J_setSize, sizeof(int));
	randctx **tempCTX = (randctx **) calloc(J_setSize, sizeof(randctx*));
	int partyID_Partner = 1 - partyID_Own;
	int result, *resultArray;


	for(j = 0; j < numCircuits; j ++)
	{
		if(0x01 == J_set[j])
		{
			tempCTX[k] = (randctx*) calloc(1, sizeof(randctx));
			setIsaacContextFromSeed(tempCTX[k], revealStruct -> revealedSeeds[j]);
			idList[k] = j;
			k ++;
		}
	}

	resultArray = (int * ) calloc(J_setSize, sizeof(int));
	// Having got all the seeds, now construct our own version of the given circuits, then compare our
	// versions to the version give by the other party.
	#pragma omp parallel for default(shared) private(j, k, tempGarbleCircuit)
	for(j = 0; j < J_setSize; j ++)
	{
		k = idList[j];

		tempGarbleCircuit = readInCircuit_FromRaw_HKE_2013(tempCTX[j], rawInputCircuit, NaorPinkasInputs[k],
														revealStruct -> outputWireShares[k], params, partyID_Partner);

		resultArray[j] = compareCircuit(rawInputCircuit, circuitsArrayPartner[k], tempGarbleCircuit);


		freeTempGarbleCircuit(tempGarbleCircuit);
	}

	result = 0;
	for(j = 0; j < J_setSize; j ++)
	{
		result |= resultArray[j];
	}

	return result;
}



int HKE_Step5_Checks(int writeSocket, int readSocket, struct RawCircuit *rawInputCircuit, struct Circuit **circuitsArray_Partner,
				struct eccPoint *C, struct jSetRevealHKE *partnerReveals, struct eccPoint ***NaorPinkasInputs,
				struct eccPoint ***NaorPinkasInputs_Partner, struct HKE_Output_Struct_Builder *outputStruct_Partner,
				struct builderInputCommitStruct *commitStruct, struct builderInputCommitStruct *partnerCommitStruct,
				unsigned char *inputBitsOwn, unsigned char *J_SetOwn, unsigned char *J_setPartner,
				int numCircuits, struct DDH_Group *groupPartner, struct eccParams *params, int partyID)
{
	struct eccPoint ***AltNP_Inputs_Partner;
	unsigned char *commBuffer;
	int commBufferLen, tempOffset = 0, circuitsCorrect = 0, outputsVerified = 0, validCommitments = 0;


	AltNP_Inputs_Partner = computeNaorPinkasInputsForJSet(C, partnerReveals -> aListRevealed, circuitsArray_Partner[0] -> numInputsBuilder,
														numCircuits, params, J_SetOwn);

	commBuffer = decommitToCheckCircuitInputs_Builder(commitStruct, inputBitsOwn, J_setPartner, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);

	commBuffer = receiveBoth(readSocket, commBufferLen);
	validCommitments = decommitToCheckCircuitInputs_Exec(partnerCommitStruct, partnerReveals, AltNP_Inputs_Partner,
														commBuffer, J_SetOwn, numCircuits,
														circuitsArray_Partner[0] -> numInputsBuilder, &tempOffset);
	free(commBuffer);


	circuitsCorrect = HKE_performCircuitChecks(circuitsArray_Partner, rawInputCircuit, AltNP_Inputs_Partner, partnerReveals,
											params, J_SetOwn, numCircuits / 2, numCircuits, partyID);


	outputsVerified = verifyRevealedOutputs(outputStruct_Partner, partnerReveals, J_SetOwn, numCircuits,
											rawInputCircuit -> numOutputs, groupPartner);

	printf("\nValid Commitment : %d\n", validCommitments);
	fflush(stdout);
	printf("Circuits Correct : %d\n", circuitsCorrect);
	fflush(stdout);
	printf("Outputs Verified : %d\n", outputsVerified);
	fflush(stdout);


	return (validCommitments | outputsVerified | circuitsCorrect);
}



// NOTE : PartyID is 1 for P1, 0 for P2. For now you're just going to have to accept this and move on.
void run_HKE_2013_CnC_OT(int writeSocket, int readSocket, struct RawCircuit *rawInputCircuit, struct idAndValue *startOfInputChain, randctx *ctx, int partyID)
{
	struct Circuit **circuitsArray_Own, **circuitsArray_Partner;
	int i, commBufferWriteLen = 0, commBufferReadLen = 0, J_setSize = 0, j, k;

	struct timespec ext_t_0, ext_t_1;
	struct timespec int_t_0, int_t_1;
	clock_t ext_c_0, ext_c_1;
	clock_t int_c_0, int_c_1;

	unsigned char *commBufferWrite, *commBufferRead;
	unsigned char **J_setPair, *J_SetOwn, *J_setPartner, *inputBitsOwn, ***OT_Inputs, **OT_Outputs;
	unsigned char **secureEqualityInputs, *binaryOutput;
	struct secureEqualityCommitments *secEqualityCommits_Own, *secEqualityCommits_Partner;
	struct eccParams *params;

	struct OT_NP_Receiver_Query **queries_Own;
	struct eccPoint ***NaorPinkasInputs, **queries_Partner;
	struct eccPoint ***NP_Inputs_Partner, *C, *cTilde;
	struct builderInputCommitStruct *commitStruct, *partnersCommitStruct;
	mpz_t **aList, **partnerSecretList;

	struct HKE_Output_Struct_Builder *outputStruct_Own, *outputStruct_Partner;
	struct jSetRevealHKE *partnerReveals;
	struct DDH_Group *groupOwn, *groupPartner;
	int numCircuits = stat_SecParam + 6; // Hardcoded laziness for S = 40.
	int tempLength = 0, bufferOffset = 0;
	int jSetChecks = 0, logChecks, numOwnInputs;

	ub4 **circuitSeeds = (ub4 **) calloc(numCircuits, sizeof(ub4*));
	randctx **circuitCTXs = (randctx **) calloc(numCircuits, sizeof(randctx*));
	gmp_randstate_t *state;



	initRandGen();
	state = seedRandGenFromISAAC(ctx);
	// Really not happy with this, don't think it provides good enough security.
	groupOwn = get_128_Bit_Group(*state);
	params = initBrainpool_256_Curve();

	for(i = 0; i < numCircuits; i ++)
	{
		circuitCTXs[i] = (randctx*) calloc(1, sizeof(randctx));
		circuitSeeds[i] = getIsaacContext(circuitCTXs[i]);
	}


	printf("Partner has connected to us.\n");

	ext_t_0 = timestamp();
	ext_c_0 = clock();
	int_t_0 = timestamp();
	int_c_0 = clock();

	outputStruct_Own = getOutputSecretsAndScheme(rawInputCircuit -> numOutputs, numCircuits, *state, groupOwn);
	C = setup_OT_NP_Sender(params, *state);
	cTilde = exchangeC_ForNaorPinkas(writeSocket, readSocket, C);

	if(0 == partyID)
	{
		numOwnInputs = rawInputCircuit -> numInputs_P2;
	}
	else
	{
		numOwnInputs = rawInputCircuit -> numInputs_P1;
	}

	aList = getNaorPinkasInputs(numOwnInputs, numCircuits, *state, params);
	NaorPinkasInputs = computeNaorPinkasInputs(cTilde, aList, numOwnInputs, numCircuits, params);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Circuit building preparation complete");
	printAndZeroBothCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	// Build the circuits that you own.
	circuitsArray_Own = buildAll_HKE_Circuits_Alt(rawInputCircuit, NaorPinkasInputs, outputStruct_Own, params,
											circuitCTXs, numCircuits, partyID);

	circuitsArray_Partner = (struct Circuit **) calloc(numCircuits, sizeof(struct Circuit *));
	
	// From the circuits you just created grab the OT inputs.
	OT_Inputs = getAllInputKeysSymm(circuitsArray_Own, numCircuits, partyID);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Circuit building complete");
	printAndZeroBothCounters();

	int_t_0 = timestamp();
	int_c_0 = clock();

	if(0 == partyID)
	{
		for(i = 0; i < numCircuits; i++)
		{
			sendCircuit(writeSocket, readSocket, circuitsArray_Own[i]);
		}
		for(i = 0; i < numCircuits; i ++)
		{
			circuitsArray_Partner[i] = receiveFullCircuit(writeSocket, readSocket);
			setCircuitsInputs_Values(startOfInputChain, circuitsArray_Partner[i], 0xFF);
		}
	}
	else
	{
		for(i = 0; i < numCircuits; i ++)
		{
			circuitsArray_Partner[i] = receiveFullCircuit(writeSocket, readSocket);
			setCircuitsInputs_Values(startOfInputChain, circuitsArray_Partner[i], 0xFF);
		}
		for(i = 0; i < numCircuits; i++)
		{
			sendCircuit(writeSocket, readSocket, circuitsArray_Own[i]);
		}
	}

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Circuits exchanged");
	printAndZeroBothCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	// Send the public commitments for the Verified Secret Sharing Scheme and exchange groups.
	outputStruct_Partner = exchangePublicBoxesOfSchemes(writeSocket, readSocket, outputStruct_Own, rawInputCircuit -> numOutputs, numCircuits);
	sendDDH_Group(writeSocket, readSocket, groupOwn);
	groupPartner = receiveDDH_Group(writeSocket, readSocket);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Exchanging Secret scheme params.");
	printAndZeroBothCounters();

	// Having received the circuits we now have the OT.
	// Note we do this after the circuits have been built and sent.


	int_t_0 = timestamp();
	int_c_0 = clock();

	inputBitsOwn = convertChainIntoArray(startOfInputChain, circuitsArray_Own[0] -> numInputsBuilder);

	// Perform all the Oblivious Transfer stuff.
	queries_Own = NaorPinkas_OT_Produce_Queries(circuitsArray_Partner[0] -> numInputsExecutor, inputBitsOwn, state, params, cTilde);
	queries_Partner = NaorPinkas_OT_Exchange_Queries(writeSocket, readSocket, circuitsArray_Partner[0] -> numInputsExecutor,
													circuitsArray_Own[0] -> numInputsExecutor, queries_Own);
	NaorPinkas_OT_Sender_Transfer(writeSocket, readSocket, circuitsArray_Own[0] -> numInputsExecutor, OT_Inputs, state, numCircuits, queries_Partner, params, C);
	OT_Outputs = NaorPinkas_OT_Receiver_Transfer(writeSocket, readSocket, circuitsArray_Partner[0] -> numInputsExecutor, inputBitsOwn, state, numCircuits, queries_Own, params, cTilde);

	setInputsFromNaorPinkas(circuitsArray_Partner, OT_Outputs, numCircuits, partyID);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "OT");
	printAndZeroBothCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	// Each party now commits to their input values.
	commitStruct = makeCommitmentsBuilder(ctx, circuitsArray_Own, state, numCircuits);
	commBufferWrite = serialiseC_Boxes(commitStruct, &commBufferWriteLen);
	sendBoth(writeSocket, commBufferWrite, commBufferWriteLen);
	free(commBufferWrite);

	commBufferReadLen = 0;
	bufferOffset = 0;
	commBufferRead = receiveBoth(readSocket, commBufferReadLen);
	partnersCommitStruct = deserialiseC_Boxes(commBufferRead, state, &bufferOffset);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Commitments made.");
	printAndZeroBothCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();


	randctx *tempCTX;
	tempCTX = getRandCtxByCoinToss(writeSocket, readSocket, ctx, state, partyID);
	J_setPair = getJ_Set(tempCTX, partyID, numCircuits);
	J_SetOwn = J_setPair[0];
	J_setPartner = J_setPair[1];


	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "J set exchange.");
	printAndZeroBothCounters();


	int_t_0 = timestamp();
	int_c_0 = clock();

	commBufferWrite = jSetRevealSerialise(circuitsArray_Own, startOfInputChain, NaorPinkasInputs, aList, inputBitsOwn, outputStruct_Own, circuitSeeds, J_setPartner,
									circuitsArray_Own[0] -> numInputsBuilder, rawInputCircuit -> numOutputs, numCircuits, &commBufferWriteLen);
	sendBoth(writeSocket, commBufferWrite, commBufferWriteLen);
	free(commBufferWrite);

	commBufferRead = receiveBoth(readSocket, commBufferReadLen);
	partnerReveals = jSetRevealDeserialise(circuitsArray_Partner, commBufferRead, J_SetOwn, circuitsArray_Partner[0] -> numInputsBuilder,
										rawInputCircuit -> numOutputs, numCircuits);
	free(commBufferRead);


	commBufferWrite = serialiseSeeds(circuitSeeds, J_setPartner, numCircuits, &commBufferWriteLen);
	sendBoth(writeSocket, commBufferWrite, commBufferWriteLen);
	free(commBufferWrite);

	commBufferRead = receiveBoth(readSocket, commBufferReadLen);
	partnerReveals -> revealedSeeds = deserialiseSeeds(commBufferRead, J_SetOwn, numCircuits);
	free(commBufferRead);


	NP_Inputs_Partner = computeNaorPinkasInputsForJSet(C, partnerReveals -> aListRevealed, circuitsArray_Partner[0] -> numInputsBuilder,
														numCircuits, params, J_SetOwn);

	jSetChecks = HKE_Step5_Checks(writeSocket, readSocket, rawInputCircuit, circuitsArray_Partner, C, partnerReveals, NaorPinkasInputs, NP_Inputs_Partner,
								outputStruct_Partner, commitStruct, partnersCommitStruct,
								inputBitsOwn, J_SetOwn, J_setPartner, numCircuits, groupPartner, params, partyID);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Initial J-Set checks");
	printAndZeroBothCounters();


	setBuildersInputsNaorPinkas(circuitsArray_Partner, rawInputCircuit, partnerReveals -> builderInputsEval,
								J_SetOwn, numCircuits, partyID);

	int_t_0 = timestamp();
	int_c_0 = clock();


	bufferOffset = 0;
	commBufferWrite = Step5_CalculateLogarithms(NaorPinkasInputs, aList, queries_Own, params, inputBitsOwn, J_setPartner, numCircuits, rawInputCircuit -> numInputs_P2, &commBufferWriteLen);
	commBufferRead = sendReceiveExchange(writeSocket, readSocket, commBufferWrite,
										commBufferWriteLen, &commBufferReadLen, partyID);
	logChecks = Step5_CheckLogarithms(commBufferRead, partnerReveals -> builderInputsEval, queries_Partner, params,
									J_SetOwn, numCircuits, rawInputCircuit -> numInputs_P2, &bufferOffset);
	free(commBufferWrite);
	free(commBufferRead);
	printf("Logarithm Checks : %d\n\n", logChecks);


	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Logarithm Checks");
	printAndZeroBothCounters();


	printf("\nEvaluating Circuits ");
	fflush(stdout);
	for(i = 0; i < numCircuits; i ++)
	{
		if(0x00 == J_SetOwn[i])
		{
			printf("%d, ", i);
			fflush(stdout);
			runCircuitExec( circuitsArray_Partner[i], 0, 0 );
		}
	}
	printf("\n");


	int_t_0 = timestamp();
	int_c_0 = clock();

	binaryOutput = HKE_OutputDetermination(writeSocket, readSocket, state, circuitsArray_Partner, rawInputCircuit, groupPartner,
										partnerReveals, outputStruct_Own, outputStruct_Partner, numCircuits, J_SetOwn, &tempLength, partyID);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Output determination");
	printAndZeroBothCounters();


	printf("Candidate Output binary : ");
	for(i = 0; i < tempLength; i ++)
	{
		printf("%X", binaryOutput[i]);
	}
	printf("\n");

	ext_c_1 = clock();
	ext_t_1 = timestamp();

	printTiming(&ext_t_0, &ext_t_1, ext_c_0, ext_c_1, "Total time without connection setup");

	/*
	for(i = 0; i < numCircuits; i ++)
	{
		freeCircuitStruct(circuitsArray_Own[i], 0);
		freeCircuitStruct(circuitsArray_Partner[i], 0);
	}
	*/
}



void runP1_HKE_2013(struct RawCircuit *rawInputCircuit, struct idAndValue *startOfInputChain, char *portNumStr, randctx *ctx)
{
	struct sockaddr_in destWrite, destRead;
	int writeSocket, readSocket, mainWriteSock, mainReadSock;
	int readPort = atoi(portNumStr), writePort = readPort + 1;


	set_up_server_socket(destRead, readSocket, mainReadSock, readPort);
	set_up_server_socket(destWrite, writeSocket, mainWriteSock, writePort);


	run_HKE_2013_CnC_OT(writeSocket, readSocket, rawInputCircuit, startOfInputChain, ctx, 0);

	close_server_socket(writeSocket, mainWriteSock);
	close_server_socket(readSocket, mainReadSock);


	freeRawCircuit(rawInputCircuit);
	free_idAndValueChain(startOfInputChain);

	printBothTotalCounters();
}




void runP2_HKE_2013(struct RawCircuit *rawInputCircuit, struct idAndValue *startOfInputChain, char *ipAddress, char *portNumStr, randctx *ctx)
{
	struct sockaddr_in serv_addr_write, serv_addr_read;
	int writeSocket, readSocket, mainWriteSock, mainReadSock;
	int writePort = atoi(portNumStr), readPort = writePort + 1;


	set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr_write);
	set_up_client_socket(readSocket, ipAddress, readPort, serv_addr_read);


	run_HKE_2013_CnC_OT(writeSocket, readSocket, rawInputCircuit, startOfInputChain, ctx, 1);


	close_client_socket(readSocket);
	close_client_socket(writeSocket);


	freeRawCircuit(rawInputCircuit);
	free_idAndValueChain(startOfInputChain);

	printBothTotalCounters();
}


