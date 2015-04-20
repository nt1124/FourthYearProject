void printSecrets(struct HKE_Output_Struct_Builder *outputStruct_Own, int numOutputs)
{
	int i, j;

	for(i = 0; i < numOutputs; i ++)
	{
		gmp_printf("%d - %Zd\n", i, outputStruct_Own -> scheme0Array[i] -> secret);
		gmp_printf("%d - %Zd\n", i, outputStruct_Own -> scheme1Array[i] -> secret);
	}
	printf("\n+++\n\n");
	fflush(stdout);

}


struct Circuit **buildAll_HKE_Circuits(struct RawCircuit *rawInputCircuit, struct idAndValue *startOfInputChain,
									struct eccPoint *C, struct eccPoint ***NaorPinkasInputs,
									struct HKE_Output_Struct_Builder *outputStruct_Own, struct eccParams *params,
									randctx *globalRandCTX, randctx **circuitCTXs, ub4 **circuitSeeds,
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
		circuitsArray_Own[j] = readInCircuit_FromRaw_HKE_2013(circuitCTXs[j], rawInputCircuit, C, NaorPinkasInputs[j], outputKeysLocals, params, partyID);

		for(i = 0; i < 2 * rawInputCircuit -> numOutputs; i ++)
		{
			mpz_clear(outputKeysLocals[i]);
		}
		free(outputKeysLocals);
	}


	for(j = 0; j < numCircuits; j++)
	{
		start = startOfInputChain;
		setCircuitsInputs_Hardcode(start, circuitsArray_Own[j], 0xFF);
	}


	return circuitsArray_Own;
}




int HKE_performCircuitChecks(struct Circuit **circuitsArrayPartner, struct RawCircuit *rawInputCircuit, struct eccPoint *cTilde,
							struct eccPoint ***NaorPinkasInputs, struct jSetRevealHKE *revealStruct, struct eccParams *params,
							unsigned char *J_set, int J_setSize, int numCircuits, int partyID_Own)
{
	struct Circuit *tempGarbleCircuit;
	int i, j, temp = 0, k = 0, *idList = (int*) calloc(J_setSize, sizeof(int));
	randctx **tempCTX = (randctx **) calloc(J_setSize, sizeof(randctx*));
	int partyID_Partner = 1 - partyID_Own;


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

	#pragma omp parallel for default(shared) private(i, j, k, tempGarbleCircuit) reduction(|:temp) 
	for(j = 0; j < J_setSize; j ++)
	{
		k = idList[j];

		tempGarbleCircuit = readInCircuit_FromRaw_HKE_2013(tempCTX[j], rawInputCircuit, cTilde, NaorPinkasInputs[k], revealStruct -> outputWireShares[k], params, partyID_Partner);

		temp |= compareCircuit(rawInputCircuit, circuitsArrayPartner[k], tempGarbleCircuit);

		freeTempGarbleCircuit(tempGarbleCircuit);
	}

	return temp;
}



int HKE_Step5_Checks(int writeSocket, int readSocket, struct RawCircuit *rawInputCircuit, struct Circuit **circuitsArray_Partner,
				struct eccPoint *C, struct jSetRevealHKE *partnerReveals, struct eccPoint ***NaorPinkasInputs,
				struct HKE_Output_Struct_Builder *outputStruct_Own, struct HKE_Output_Struct_Builder *outputStruct_Partner,
				struct builderInputCommitStruct *commitStruct, struct builderInputCommitStruct *partnerCommitStruct,
				unsigned char *inputBitsOwn, unsigned char *J_SetOwn, unsigned char *J_setPartner,
				int numCircuits, struct DDH_Group *groupPartner, struct eccParams *params, int partyID)
{
	struct eccPoint ***NaorPinkasInputs_Partner;
	unsigned char *commBuffer;
	int commBufferLen, tempOffset = 0, circuitsCorrect = 0, outputsVerified = 0, validCommitments = 0;


	NaorPinkasInputs_Partner = computeNaorPinkasInputsForJSet(C, partnerReveals -> aListRevealed, rawInputCircuit -> numInputs_P1, numCircuits, params, J_SetOwn);


	commBuffer = decommitToCheckCircuitInputs_Builder(commitStruct, inputBitsOwn, J_setPartner, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);
	commBuffer = receiveBoth(readSocket, commBufferLen);
	validCommitments = decommitToCheckCircuitInputs_Exec(partnerCommitStruct, partnerReveals, NaorPinkasInputs_Partner, commBuffer, J_SetOwn, numCircuits,
														rawInputCircuit -> numInputs_P2, &tempOffset);
	free(commBuffer);


	circuitsCorrect = HKE_performCircuitChecks(circuitsArray_Partner, rawInputCircuit, C, NaorPinkasInputs_Partner, partnerReveals,
											params, J_SetOwn, numCircuits / 2, numCircuits, partyID);
	outputsVerified = verifyRevealedOutputs(outputStruct_Partner, partnerReveals, J_setPartner, numCircuits, rawInputCircuit -> numOutputs, groupPartner);
	
	printf("\nValid Commitment : %d\n", validCommitments);
	printf("Circuits correct : %d\n", circuitsCorrect);
	printf("Outputs verified : %d\n", outputsVerified);


	return (validCommitments | outputsVerified | circuitsCorrect);
}



// NOTE : PartyID is 1 for P1, 0 for P2. For now you're just going to have to accept this and move on.
void run_HKE_2013_CnC_OT(int writeSocket, int readSocket, struct RawCircuit *rawInputCircuit, struct idAndValue *startOfInputChain, randctx *ctx, int partyID)
{
	struct Circuit **circuitsArray_Own, **circuitsArray_Partner;
	int i, commBufferLen = 0, J_setSize = 0, j, k;

	struct timespec ext_t_0, ext_t_1;
	struct timespec int_t_0, int_t_1;
	clock_t ext_c_0, ext_c_1;
	clock_t int_c_0, int_c_1;

	unsigned char *commBuffer, *J_SetOwn, *J_setPartner, *inputBitsOwn, ***OT_Inputs, **OT_Outputs;
	unsigned char **secureEqualityInputs, *hexOutputs;
	struct secureEqualityCommitments *secEqualityCommits_Own, *secEqualityCommits_Partner;
	struct eccParams *params;

	struct OT_NP_Receiver_Query **queries_Own;
	struct eccPoint ***NaorPinkasInputs, ***NaorPinkasInputs_Partner, **queries_Partner, *C, *cTilde;
	struct builderInputCommitStruct *commitStruct, *partnersCommitStruct;
	mpz_t **aList, **partnerSecretList;

	struct HKE_Output_Struct_Builder *outputStruct_Own, *outputStruct_Partner;
	struct jSetRevealHKE *partnerReveals;
	struct DDH_Group *groupOwn, *groupPartner;
	int numCircuits = stat_SecParam, tempLength = 0, bufferOffset = 0;
	int jSetChecks = 0, logChecks; 

	ub4 **circuitSeeds = (ub4 **) calloc(numCircuits, sizeof(ub4*));
	randctx **circuitCTXs = (randctx **) calloc(numCircuits, sizeof(randctx*));
	gmp_randstate_t *state;



	initRandGen();
	state = seedRandGenFromISAAC(ctx);
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
	aList = getNaorPinkasInputs(rawInputCircuit -> numInputs_P1, numCircuits, *state, params);
	NaorPinkasInputs = computeNaorPinkasInputs(cTilde, aList, rawInputCircuit -> numInputs_P1, numCircuits, params);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Circuit building preparation complete");


	int_t_0 = timestamp();
	int_c_0 = clock();

	// Build the circuits that you own.
	circuitsArray_Own = buildAll_HKE_Circuits(rawInputCircuit, startOfInputChain, C, NaorPinkasInputs, outputStruct_Own, params,
											ctx, circuitCTXs, circuitSeeds, numCircuits, partyID);

	circuitsArray_Partner = (struct Circuit **) calloc(numCircuits, sizeof(struct Circuit *));
	
	// From the circuits you just created grab the OT inputs.
	OT_Inputs = getAllInputKeysSymm(circuitsArray_Own, numCircuits, partyID);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Circuit building complete");

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

	// Send the public commitments for the Verified Secret Sharing Scheme and exchange groups.
	outputStruct_Partner = exchangePublicBoxesOfSchemes(writeSocket, readSocket, outputStruct_Own, rawInputCircuit -> numOutputs, numCircuits);
	sendDDH_Group(writeSocket, readSocket, groupOwn);
	groupPartner = receiveDDH_Group(writeSocket, readSocket);

	// Having received the circuits we now have the OT.
	// Note we do this after the circuits have been built and sent.
	inputBitsOwn = convertChainIntoArray(startOfInputChain, circuitsArray_Own[0] -> numInputsBuilder);

	int_t_0 = timestamp();
	int_c_0 = clock();

	// Perform all the Oblivious Transfer stuff.
	queries_Own = NaorPinkas_OT_Produce_Queries(rawInputCircuit -> numInputs_P1, inputBitsOwn, state, params, cTilde);
	queries_Partner = NaorPinkas_OT_Exchange_Queries(writeSocket, readSocket, rawInputCircuit -> numInputs_P1, queries_Own);
	NaorPinkas_OT_Sender_Transfer(writeSocket, readSocket, rawInputCircuit -> numInputs_P1, OT_Inputs, state, numCircuits, queries_Partner, params, C);
	OT_Outputs = NaorPinkas_OT_Receiver_Transfer(writeSocket, readSocket, rawInputCircuit -> numInputs_P1, inputBitsOwn, state, numCircuits, queries_Own, params, cTilde);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "OT");


	int_t_0 = timestamp();
	int_c_0 = clock();

	// Each party now commits to their input values.
	commitStruct = makeCommitmentsBuilder(ctx, circuitsArray_Own, state, numCircuits);
	commBuffer = serialiseC_Boxes(commitStruct, &tempLength);
	sendBoth(writeSocket, commBuffer, tempLength);
	free(commBuffer);

	commBufferLen = 0;
	bufferOffset = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	partnersCommitStruct = deserialiseC_Boxes(commBuffer, commitStruct -> iMax, commitStruct -> jMax, state, &bufferOffset);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Commitments made");


	setInputsFromNaorPinkas(circuitsArray_Partner, OT_Outputs, numCircuits, partyID);

	J_SetOwn = generateJ_Set(numCircuits);
	J_setPartner = getPartnerJ_Set(writeSocket, readSocket, J_SetOwn, numCircuits / 2, numCircuits);


	int_t_0 = timestamp();
	int_c_0 = clock();

	commBuffer = jSetRevealSerialise(NaorPinkasInputs, aList, inputBitsOwn, outputStruct_Own, circuitSeeds, J_setPartner, rawInputCircuit -> numInputs_P1, rawInputCircuit -> numOutputs, numCircuits, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);
	commBuffer = receiveBoth(readSocket, commBufferLen);
	partnerReveals = jSetRevealDeserialise(commBuffer, J_SetOwn, rawInputCircuit -> numInputs_P1, rawInputCircuit -> numOutputs, numCircuits);
	free(commBuffer);



	jSetChecks = HKE_Step5_Checks(writeSocket, readSocket, rawInputCircuit, circuitsArray_Partner, C, partnerReveals, NaorPinkasInputs,
								outputStruct_Own, outputStruct_Partner, commitStruct, partnersCommitStruct,
								inputBitsOwn, J_SetOwn, J_setPartner, numCircuits, groupPartner, params, partyID);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Initial J-Set checks");


	/*
	bufferOffset = 0;
	commBuffer = Step5_CalculateLogarithms(NaorPinkasInputs, aList, queries_Own, params, inputBitsOwn, J_setPartner, numCircuits, rawInputCircuit -> numInputs_P2, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);

	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	logChecks = Step5_CheckLogarithms(commBuffer, partnerReveals -> builderInputsEval, queries_Partner, params, J_SetOwn, numCircuits, rawInputCircuit -> numInputs_P2, &bufferOffset);
	printf("Logarithm Checks : %d\n\n", logChecks);
	*/
	if(0 == partyID)
	{
		bufferOffset = 0;
		commBuffer = Step5_CalculateLogarithms(NaorPinkasInputs, aList, queries_Own, params, inputBitsOwn, J_setPartner, numCircuits, rawInputCircuit -> numInputs_P2, &commBufferLen);
		sendBoth(writeSocket, commBuffer, commBufferLen);
		free(commBuffer);

		commBufferLen = 0;
		commBuffer = receiveBoth(readSocket, commBufferLen);
		logChecks = Step5_CheckLogarithms(commBuffer, partnerReveals -> builderInputsEval, queries_Partner, params, J_SetOwn, numCircuits, rawInputCircuit -> numInputs_P2, &bufferOffset);
		printf("Logarithm Checks : %d\n\n", logChecks);
	}
	else
	{
		commBufferLen = 0;
		commBuffer = receiveBoth(readSocket, commBufferLen);
		logChecks = Step5_CheckLogarithms(commBuffer, partnerReveals -> builderInputsEval, queries_Partner, params, J_SetOwn, numCircuits, rawInputCircuit -> numInputs_P2, &bufferOffset);
		printf("Logarithm Checks : %d\n\n", logChecks);

		bufferOffset = 0;
		commBuffer = Step5_CalculateLogarithms(NaorPinkasInputs, aList, queries_Own, params, inputBitsOwn, J_setPartner, numCircuits, rawInputCircuit -> numInputs_P2, &commBufferLen);
		sendBoth(writeSocket, commBuffer, commBufferLen);
		free(commBuffer);
	}

	for(i = 0; i < numCircuits; i ++)
	{
		if(0x00 == J_SetOwn[i])
		{
			runCircuitExec( circuitsArray_Partner[i], 0, 0 );
		}
		// printOutputHexString(circuitsArray_Partner[i]);
	}

	printMajorityOutputAsBinary(circuitsArray_Partner, numCircuits, J_SetOwn);


	// printSecrets(outputStruct_Own, rawInputCircuit -> numOutputs);

	hexOutputs = HKE_OutputDetermination(writeSocket, readSocket, state, circuitsArray_Partner, rawInputCircuit, groupPartner,
										partnerReveals, outputStruct_Own, outputStruct_Partner, numCircuits, J_SetOwn, &commBufferLen, partyID);


	printf("Candidate Output binary : ");
	for(i = 0; i < commBufferLen; i ++)
	{
		printf("%X", hexOutputs[i]);
	}
	printf("\n");

	ext_c_1 = clock();
	ext_t_1 = timestamp();

	printTiming(&ext_t_0, &ext_t_1, ext_c_0, ext_c_1, "\nTotal time without connection setup");

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
}


