


struct secCompBuilderOutput *SC_DetectCheatingBuilder_HKE(int writeSocket, int readSocket, struct RawCircuit *rawInputCircuit,
													struct idAndValue *startOfInputChain, unsigned char *delta, int lengthDelta,
													struct eccPoint ***NP_consistentInputsCheck, mpz_t **aList,
													struct eccPoint *C, struct eccPoint *cTilde,
													int checkStatSecParam, gmp_randstate_t *state, randctx *ctx)
{
	struct Circuit **circuitsArray_Own, **circuitsArray_Partner;
	struct eccParams *params;
	struct secCompBuilderOutput *returnStruct;

	struct HKE_Output_Struct_Builder *outputStruct_Own, *outputStruct_Partner;
	struct builderInputCommitStruct *commitStruct, *partnersCommitStruct;
	struct DDH_Group *groupOwn, *groupPartner;
	struct jSetRevealHKE *partnerReveals;

	unsigned char *commBuffer, *J_setOwn, *J_setPartner, ***OT_Inputs, *deltaExpanded, *inputBitsOwn;
	int commBufferLen = 0, i, J_setSize = 0, arrayLen = 0, bufferOffset = 0;
	int jSetChecks;

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

	inputBitsOwn = convertChainIntoArray(startOfInputChain, rawInputCircuit -> numInputs_P1);

	outputStruct_Own = getOutputSecretsAndScheme(rawInputCircuit -> numOutputs, checkStatSecParam, *state, groupOwn);
	circuitsArray_Own = buildAll_HKE_Circuits(rawInputCircuit, startOfInputChain, cTilde, NP_consistentInputsCheck, outputStruct_Own, params,
											circuitCTXs, circuitSeeds, checkStatSecParam, 1);

	for(i = 0; i < checkStatSecParam; i++)
	{
		sendCircuit(writeSocket, readSocket, circuitsArray_Own[i]);
	}
	circuitsArray_Partner = (struct Circuit **) calloc(checkStatSecParam, sizeof(struct Circuit*));
	for(i = 0; i < checkStatSecParam; i ++)
	{
		circuitsArray_Partner[i] = receiveFullCircuit(writeSocket, readSocket);
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


	// THIS SHOULD ONLY BE UNCOMMENTED FOR TESTING, MEANS WE CAN GET AN EASY DELTA = DELTA'
	// delta = (unsigned char *) calloc(16, sizeof(unsigned char));
	OT_Inputs = getCheckCircuitOT_Inputs(circuitsArray_Own, delta, checkStatSecParam, lengthDelta);

	int_t_0 = timestamp();
	int_c_0 = clock();

	full_CnC_OT_Sender_ECC(writeSocket, readSocket, lengthDelta,
						OT_Inputs, state, checkStatSecParam, 1024);

	commBuffer = getK0_AndDelta(circuitsArray_Own, delta, rawInputCircuit -> numInputs_P1, checkStatSecParam, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "subOT - Sender");


	J_setOwn = generateJ_Set(stat_SecParam);
	J_setPartner = getPartnerJ_Set(writeSocket, readSocket, J_setOwn, stat_SecParam / 2, stat_SecParam);


	commBuffer = jSetRevealSerialise(NP_consistentInputsCheck, aList, inputBitsOwn, outputStruct_Own, circuitSeeds, J_setPartner,
									rawInputCircuit -> numInputs_P1, rawInputCircuit -> numOutputs, checkStatSecParam, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);
	commBuffer = receiveBoth(readSocket, commBufferLen);
	partnerReveals = jSetRevealDeserialise(commBuffer, J_setOwn, rawInputCircuit -> numInputs_P2, rawInputCircuit -> numOutputs, checkStatSecParam);
	free(commBuffer);


	jSetChecks = HKE_Step5_Checks(writeSocket, readSocket, rawInputCircuit, circuitsArray_Partner, C, partnerReveals, NP_consistentInputsCheck,
								outputStruct_Own, outputStruct_Partner, commitStruct, partnersCommitStruct,
								inputBitsOwn, J_setOwn, J_setPartner, checkStatSecParam, groupPartner, params, 1);
	/*
	builderInputs =  computeBuilderInputs(public_inputs, secret_inputs,
										J_set, J_setSize, startOfInputChain, 
										params, &arrayLen);

	commBuffer = serialise_ECC_Point_Array(builderInputs, arrayLen, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);
	*/

	/*

	returnStruct = getSecCompReturnStruct_L_2013_B(public_inputs, builderInputs, J_set, J_setSize);
	*/

	return returnStruct;
}


