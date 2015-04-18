struct secCompBuilderOutput_HKE *getSecCompReturnStruct_L_2013_B_HKE(unsigned char *J_set, int J_setSize)
{
	struct secCompBuilderOutput_HKE *returnStruct = (struct secCompBuilderOutput_HKE *) calloc(1, sizeof(struct secCompBuilderOutput_HKE));


	returnStruct -> J_set = J_set;
	returnStruct -> J_setSize = J_setSize;

	return returnStruct;
}


unsigned char ***getCheckCircuitOT_Inputs_HKE(struct Circuit **circuitsArray, unsigned char *delta,
										int checkStatSecParam, int lengthDelta)
{
	struct wire *tempWire;
	unsigned char ***OT_Inputs = (unsigned char ***) calloc(2, sizeof(unsigned char**));
	unsigned char goodBit, **XOR_Sum = (unsigned char **) calloc(checkStatSecParam, sizeof(unsigned char *));
	int i, j, k, OT_Index = 0, numInputsB;


	numInputsB = circuitsArray[0] -> numInputsBuilder;
	OT_Inputs[0] = (unsigned char **) calloc(lengthDelta * checkStatSecParam, sizeof(unsigned char*));
	OT_Inputs[1] = (unsigned char **) calloc(lengthDelta * checkStatSecParam, sizeof(unsigned char*));

	for(i = 0; i < checkStatSecParam; i ++)
	{
		XOR_Sum[i] = (unsigned char *) calloc(16, sizeof(unsigned char));
		tempWire = circuitsArray[i] -> gates[numInputsB] -> outputWire;
		memcpy(XOR_Sum[i], tempWire -> outputGarbleKeys -> key1, 16);
	}

	for(i = 0; i < lengthDelta - 1; i ++)
	{
		goodBit = getBitFromCharArray(delta, i);

		for(j = 0; j < checkStatSecParam; j ++)
		{
			OT_Inputs[0][OT_Index] = generateRandBytes(16, 16);
			OT_Inputs[1][OT_Index] = generateRandBytes(16, 16);

			for(k = 0; k < 16; k ++)
			{
				XOR_Sum[j][k] ^= OT_Inputs[goodBit][OT_Index][k];
			}

			OT_Index ++;
		}
	}

	goodBit = getBitFromCharArray(delta, lengthDelta - 1);
	for(j = 0; j < checkStatSecParam; j ++)
	{
		OT_Inputs[goodBit][OT_Index] = (unsigned char *) calloc(16, sizeof(unsigned char));
		memcpy(OT_Inputs[goodBit][OT_Index], XOR_Sum[j], 16);

		OT_Inputs[1 - goodBit][OT_Index] = generateRandBytes(16, 16);
		OT_Index ++;
	}

	return OT_Inputs;
}


unsigned char *getK0_AndDelta_HKE(struct Circuit **circuitsArray, unsigned char *delta, int numInputsB,
							int checkStatSecParam, int *commBufferLen)
{
	struct wire *tempWire;
	unsigned char *commBuffer = (unsigned char *) calloc(16 * (checkStatSecParam) + 128, sizeof(unsigned char));
	int bufferOffset = 0, i;

	for(i = 0; i < checkStatSecParam; i ++)
	{
		tempWire = circuitsArray[i] -> gates[numInputsB] -> outputWire;
		memcpy(commBuffer + bufferOffset, tempWire -> outputGarbleKeys -> key0, 16);
		bufferOffset += 16;
	}

	for(i = 0; i < 128; i ++)
	{
		commBuffer[bufferOffset] = getBitFromCharArray(delta, i);
		bufferOffset ++;
	}


	*commBufferLen = bufferOffset;

	return commBuffer;
}



struct secCompBuilderOutput_HKE *SC_DetectCheatingBuilder_HKE(int writeSocket, int readSocket, struct RawCircuit *rawInputCircuit,
															struct idAndValue *startOfInputChain, unsigned char *delta, int lengthDelta,
															struct secret_builderPRS_Keys *secret_inputs,
															int checkStatSecParam, gmp_randstate_t *state)
{
	struct Circuit **circuitsArray;
	struct public_builderPRS_Keys *public_inputs;
	struct eccParams *params;
	struct eccPoint **builderInputs;
	struct secCompBuilderOutput_HKE *returnStruct;

	unsigned char *commBuffer, *J_set, ***OT_Inputs, *deltaExpanded;
	int commBufferLen = 0, i, J_setSize = 0, arrayLen = 0;
	int partyID = 0;

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
	public_inputs = computePublicInputs(secret_inputs, params);
	circuitsArray = buildAllCircuits(rawInputCircuit, startOfInputChain, *state, checkStatSecParam, params, secret_inputs, public_inputs, circuitCTXs, circuitSeeds);


	sendPublicCommitments(writeSocket, readSocket, public_inputs, params);
	for(i = 0; i < checkStatSecParam; i++)
	{
		sendCircuit(writeSocket, readSocket, circuitsArray[i]);
	}

	// THIS SHOULD ONLY BE UNCOMMENTED FOR TESTING, MEANS WE CAN GET AN EASY DELTA = DELTA'
	// delta = (unsigned char *) calloc(16, sizeof(unsigned char));
	OT_Inputs = getCheckCircuitOT_Inputs(circuitsArray, delta, checkStatSecParam, lengthDelta);


	int_t_0 = timestamp();
	int_c_0 = clock();

	full_CnC_OT_Sender_ECC(writeSocket, readSocket, lengthDelta,
						OT_Inputs, state, checkStatSecParam, 1024);

	commBuffer = getK0_AndDelta(circuitsArray, delta, rawInputCircuit -> numInputs_P1, checkStatSecParam, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "subOT - Sender");


	J_set = builder_decommitToJ_Set(writeSocket, readSocket, circuitsArray, secret_inputs, checkStatSecParam, &J_setSize, circuitSeeds);

	builderInputs =  computeBuilderInputs(public_inputs, secret_inputs,
										J_set, J_setSize, startOfInputChain, 
										params, &arrayLen);

	commBuffer = serialise_ECC_Point_Array(builderInputs, arrayLen, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);


	returnStruct = getSecCompReturnStruct_L_2013_B_HKE(J_set, J_setSize);

	return returnStruct;
}






// NOTE : PartyID is 1 for P1, 0 for P2. For now you're just going to have to accept this and move on.
void SC_DetectCheatingBuilder_HKE(int writeSocket, int readSocket, struct RawCircuit *rawInputCircuit, struct idAndValue *startOfInputChain, randctx *ctx, int partyID)
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
	int numCircuits = 4, tempLength = 0, bufferOffset = 0;
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
	inputBitsOwn = convertChainIntoArray(startOfInputChain, circuitsArray_Own[0] -> numInputsBuilder);


	queries_Own = NaorPinkas_OT_Produce_Queries(rawInputCircuit -> numInputs_P1, inputBitsOwn, state, params, cTilde);
	queries_Partner = NaorPinkas_OT_Exchange_Queries(writeSocket, readSocket, rawInputCircuit -> numInputs_P1, queries_Own);
	NaorPinkas_OT_Sender_Transfer(writeSocket, readSocket, rawInputCircuit -> numInputs_P1, OT_Inputs, state, numCircuits, queries_Partner, params, C);
	OT_Outputs = NaorPinkas_OT_Receiver_Transfer(writeSocket, readSocket, rawInputCircuit -> numInputs_P1, inputBitsOwn, state, numCircuits, queries_Own, params, cTilde);


	// Each party now commits to their input values.
	commitStruct = makeCommitmentsBuilder(ctx, circuitsArray_Own, state, numCircuits);
	commBuffer = serialiseC_Boxes(commitStruct, &tempLength);
	sendBoth(writeSocket, commBuffer, tempLength);
	free(commBuffer);

	commBufferLen = 0;
	bufferOffset = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	partnersCommitStruct = deserialiseC_Boxes(commBuffer, commitStruct -> iMax, commitStruct -> jMax, state, &bufferOffset);



	setInputsFromNaorPinkas(circuitsArray_Partner, OT_Outputs, numCircuits, partyID);

	J_SetOwn = generateJ_Set(numCircuits);
	J_setPartner = getPartnerJ_Set(writeSocket, readSocket, J_SetOwn, numCircuits / 2, numCircuits);


	commBuffer = jSetRevealSerialise(NaorPinkasInputs, aList, inputBitsOwn, outputStruct_Own, circuitSeeds, J_setPartner, rawInputCircuit -> numInputs_P1, rawInputCircuit -> numOutputs, numCircuits, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);
	commBuffer = receiveBoth(readSocket, commBufferLen);
	partnerReveals = jSetRevealDeserialise(commBuffer, J_SetOwn, rawInputCircuit -> numInputs_P1, rawInputCircuit -> numOutputs, numCircuits);
	free(commBuffer);


	jSetChecks = HKE_Step5_Checks(writeSocket, readSocket, rawInputCircuit, circuitsArray_Partner, C, partnerReveals, NaorPinkasInputs,
								outputStruct_Own, outputStruct_Partner, commitStruct, partnersCommitStruct,
								inputBitsOwn, J_SetOwn, J_setPartner, numCircuits, groupPartner, params, partyID);


	bufferOffset = 0;
	commBuffer = Step5_CalculateLogarithms(NaorPinkasInputs, aList, queries_Own, params, inputBitsOwn, J_setPartner, numCircuits, rawInputCircuit -> numInputs_P2, &commBufferLen);
	sendBoth(writeSocket, commBuffer, tempLength);
	free(commBuffer);

	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	logChecks = Step5_CheckLogarithms(commBuffer, partnerReveals -> builderInputsEval, queries_Partner, params, J_SetOwn, numCircuits, rawInputCircuit -> numInputs_P2, &bufferOffset);
	printf("Logarithm Checks : %d\n\n", logChecks);

	for(i = 0; i < numCircuits; i ++)
	{
		runCircuitExec( circuitsArray_Partner[i], 0, 0 );
		printOutputHexString(circuitsArray_Partner[i]);
	}

	ext_c_1 = clock();
	ext_t_1 = timestamp();

	printTiming(&ext_t_0, &ext_t_1, ext_c_0, ext_c_1, "\nTotal time without connection setup");


	hexOutputs = HKE_OutputDetermination(writeSocket, readSocket, state, circuitsArray_Partner, rawInputCircuit, groupPartner,
										partnerReveals, outputStruct_Own, numCircuits, J_SetOwn, &commBufferLen);


	printf("Candidate Output : ");
	for(i = 0; i < commBufferLen; i ++)
	{
		printf("%02X", hexOutputs[i]);
	}
	printf("\n");


	for(i = 0; i < numCircuits; i ++)
	{
		freeCircuitStruct(circuitsArray_Own[i], 0);
		freeCircuitStruct(circuitsArray_Partner[i], 0);
	}

}