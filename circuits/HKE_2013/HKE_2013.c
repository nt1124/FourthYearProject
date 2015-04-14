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


	// #pragma omp parallel for private(i, j, outputKeysLocals) schedule(auto)
	for(j = 0; j < numCircuits; j++)
	{
		outputKeysLocals = getOutputKeys(outputStruct_Own, rawInputCircuit -> numOutputs, j);
		circuitsArray_Own[j] = readInCircuit_FromRaw_HKE_2013(circuitCTXs[j], rawInputCircuit, C, NaorPinkasInputs[j], outputKeysLocals, params, partyID);

		for(i = 0; i < 2*rawInputCircuit -> numOutputs; i ++)
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




int HKE_performCircuitChecks(struct Circuit **circuitsArrayPartner, struct RawCircuit *rawInputCircuit,
							struct eccPoint *cTilde, struct jSetRevealHKE *revealStruct, struct eccParams *params,
							unsigned char *J_set, int J_setSize, int numCircuits, int partyID_Own)
{
	struct Circuit *tempGarbleCircuit;
	struct eccPoint ***NaorPinkasInputs;
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

	NaorPinkasInputs = computeNaorPinkasInputsForJSet(cTilde, revealStruct -> aListRevealed, rawInputCircuit -> numInputs_P1, numCircuits, params, J_set);

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



int HKE_jSetChecks(int writeSocket, int readSocket, struct RawCircuit *rawInputCircuit, struct Circuit **circuitsArray_Partner, struct eccPoint *C, mpz_t **aList,
				ub4 **circuitSeeds, struct HKE_Output_Struct_Builder *outputStruct_Own, struct HKE_Output_Struct_Builder *outputStruct_Partner,
				unsigned char *J_setOwn, unsigned char *J_setPartner, int numCircuits, struct DDH_Group *groupPartner, struct eccParams *params, int partyID)
{
	struct jSetRevealHKE *partnerReveals;
	unsigned char *commBuffer;
	int commBufferLen, circuitsCorrect = 0, outputsVerified = 0;


	commBuffer = jSetRevealSerialise(aList, outputStruct_Own, circuitSeeds, J_setPartner, rawInputCircuit -> numInputs_P1, rawInputCircuit -> numOutputs, numCircuits, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);
	commBuffer = receiveBoth(readSocket, commBufferLen);
	partnerReveals = jSetRevealDeserialise(commBuffer, J_setOwn, rawInputCircuit -> numInputs_P1, rawInputCircuit -> numOutputs, numCircuits);


	circuitsCorrect = HKE_performCircuitChecks(circuitsArray_Partner, rawInputCircuit, C, partnerReveals, params, J_setOwn, numCircuits / 2, numCircuits, partyID);
	outputsVerified = verifyRevealedOutputs(outputStruct_Partner, partnerReveals, J_setPartner, numCircuits, rawInputCircuit -> numOutputs, groupPartner);
	printf("Circuits correct : %d\n", circuitsCorrect);
	printf("Outputs verified : %d\n\n", outputsVerified);


	return (outputsVerified | circuitsCorrect);
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

	unsigned char *commBuffer, *J_setOwn, *J_setPartner, *permedInputs, ***OT_Inputs, **OT_Outputs;
	struct eccParams *params;

	struct OT_NP_Receiver_Query **queries_Own;
	struct eccPoint ***NaorPinkasInputs, **queries_Partner, *C, *cTilde;
	struct builderInputCommitStruct *commitStruct, *partnersCommitStruct;
	mpz_t **aList;

	struct HKE_Output_Struct_Builder *outputStruct_Own, *outputStruct_Partner;
	struct jSetRevealHKE *partnerReveals;
	struct DDH_Group *groupOwn, *groupPartner;
	int numCircuits = 4, tempLength = 0, bufferOffset = 0;
	int circuitsCorrect = 0, outputsVerified = 0; 

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


	ext_t_0 = timestamp();
	ext_c_0 = clock();

	printf("Partner has connected to us.\n");

	int_t_0 = timestamp();
	int_c_0 = clock();


	outputStruct_Own = getOutputSecretsAndScheme(rawInputCircuit -> numOutputs, numCircuits, *state, groupOwn);
	C = setup_OT_NP_Sender(params, *state);
	cTilde = exchangeC_ForNaorPinkas(writeSocket, readSocket, C);
	aList = getNaorPinkasInputs(rawInputCircuit -> numInputs_P1, numCircuits, *state, params);
	NaorPinkasInputs = computeNaorPinkasInputs(cTilde, aList, rawInputCircuit -> numInputs_P1, numCircuits, params);


	circuitsArray_Own = buildAll_HKE_Circuits(rawInputCircuit, startOfInputChain, C, NaorPinkasInputs, outputStruct_Own, params,
											ctx, circuitCTXs, circuitSeeds, numCircuits, partyID);
	circuitsArray_Partner = (struct Circuit **) calloc(numCircuits, sizeof(struct Circuit *));
	
	OT_Inputs = getAllInputKeysSymm(circuitsArray_Own, numCircuits, partyID);


	for(i = 0; i < numCircuits; i++)
	{
		sendCircuit(writeSocket, readSocket, circuitsArray_Own[i]);
	}
	for(i = 0; i < numCircuits; i ++)
	{
		circuitsArray_Partner[i] = receiveFullCircuit(writeSocket, readSocket);
		setCircuitsInputs_Values(startOfInputChain, circuitsArray_Partner[i], 0xFF);
	}

	outputStruct_Partner = exchangePublicBoxesOfSchemes(writeSocket, readSocket, outputStruct_Own, rawInputCircuit -> numOutputs, numCircuits);
	sendDDH_Group(writeSocket, readSocket, groupOwn);
	groupPartner = receiveDDH_Group(writeSocket, readSocket);

	// Having received the circuits we now have the OT.
	// Note we do this after the circuits have been built and sent.
	permedInputs = getPermedInputValuesExecutor(circuitsArray_Partner, partyID);


	queries_Own = NaorPinkas_OT_Produce_Queries(rawInputCircuit -> numInputs_P1, permedInputs, state, params, cTilde);
	queries_Partner = NaorPinkas_OT_Exchange_Queries(writeSocket, readSocket, rawInputCircuit -> numInputs_P1, queries_Own);
	NaorPinkas_OT_Sender_Transfer(writeSocket, readSocket, rawInputCircuit -> numInputs_P1, OT_Inputs, state, numCircuits, queries_Partner, params, C);
	OT_Outputs = NaorPinkas_OT_Receiver_Transfer(writeSocket, readSocket, rawInputCircuit -> numInputs_P1, permedInputs, state, numCircuits, queries_Own, params, cTilde);



	// Each party now commits to their input values.
	commitStruct = makeCommitmentsBuilder(ctx, circuitsArray_Own, state, numCircuits);
	commBuffer = serialiseC_Boxes(commitStruct, &tempLength);
	sendBoth(writeSocket, commBuffer, tempLength);
	free(commBuffer);

	commBufferLen = 0;
	bufferOffset = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	partnersCommitStruct = deserialiseC_Boxes(commBuffer, commitStruct -> iMax, commitStruct -> jMax, state, &bufferOffset);


	ext_c_1 = clock();
	ext_t_1 = timestamp();


	printTiming(&ext_t_0, &ext_t_1, ext_c_0, ext_c_1, "\nTotal time without connection setup");


	setInputsFromNaorPinkas(circuitsArray_Partner, OT_Outputs, numCircuits, partyID);

	J_setOwn = generateJ_Set(numCircuits);
	J_setPartner = getPartnerJ_Set(writeSocket, readSocket, J_setOwn, numCircuits / 2, numCircuits);

	commBuffer = jSetRevealSerialise(aList, outputStruct_Own, circuitSeeds, J_setPartner, rawInputCircuit -> numInputs_P1, rawInputCircuit -> numOutputs, numCircuits, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);
	commBuffer = receiveBoth(readSocket, commBufferLen);
	partnerReveals = jSetRevealDeserialise(commBuffer, J_setOwn, rawInputCircuit -> numInputs_P1, rawInputCircuit -> numOutputs, numCircuits);


	circuitsCorrect = HKE_performCircuitChecks(circuitsArray_Partner, rawInputCircuit, C, partnerReveals, params, J_setOwn, numCircuits / 2, numCircuits, partyID);
	outputsVerified = verifyRevealedOutputs(outputStruct_Partner, partnerReveals, J_setPartner, numCircuits, rawInputCircuit -> numOutputs, groupPartner);
	printf("Circuits correct : %d\n", circuitsCorrect);
	printf("Outputs verified : %d\n\n", outputsVerified);


	for(i = 0; i < numCircuits; i ++)
	{
		runCircuitExec( circuitsArray_Partner[i], 0, 0 );
		printOutputHexString(circuitsArray_Partner[i]);
	}

	for(i = 0; i < numCircuits; i ++)
	{
		freeCircuitStruct(circuitsArray_Own[i], 0);
		freeCircuitStruct(circuitsArray_Partner[i], 0);
	}

}



void runP1_HKE_2013_CnC_OT(struct RawCircuit *rawInputCircuit, struct idAndValue *startOfInputChain, char *portNumStr, randctx *ctx)
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




void runP2_HKE_2013_CnC_OT(struct RawCircuit *rawInputCircuit, struct idAndValue *startOfInputChain, char *ipAddress, char *portNumStr, randctx *ctx)
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


