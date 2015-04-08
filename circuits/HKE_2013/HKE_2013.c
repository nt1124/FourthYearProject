struct Circuit **buildAll_HKE_Circuits(struct RawCircuit *rawInputCircuit, struct idAndValue *startOfInputChain,
									struct eccPoint *C, struct eccPoint ***NaorPinkasInputs,
									struct HKE_Output_Struct_Builder *outputStruct, struct eccParams *params,
									randctx *globalRandCTX, randctx **circuitCTXs, ub4 **circuitSeeds,
									int numCircuits, int partyID)
{
	struct Circuit **circuitsArray_Own = (struct Circuit **) calloc(numCircuits, sizeof(struct Circuit*));
	struct idAndValue *start;
	int j;


	#pragma omp parallel for private(j) schedule(auto)
	for(j = 0; j < numCircuits; j++)
	{
		circuitsArray_Own[j] = readInCircuit_FromRaw_HKE_2013(circuitCTXs[j], rawInputCircuit, C, NaorPinkasInputs[j], outputStruct, j, params, partyID);
	}


	for(j = 0; j < numCircuits; j++)
	{
		start = startOfInputChain;
		setCircuitsInputs_Hardcode(start, circuitsArray_Own[j], 0xFF);
	}


	return circuitsArray_Own;
}


void setInputsFromNaorPinkas(struct Circuit **circuitsArray_Own, unsigned char **output, int numCircuits)
{
	int i, j, iOffset, u_v_index, numInputsBuilder, numInputsExecutor;
	struct wire *tempWire;
	unsigned char value;


	numInputsBuilder = circuitsArray_Own[0] -> numInputsBuilder;
	numInputsExecutor = circuitsArray_Own[0] -> numInputsExecutor;


	for(i = numInputsBuilder; i < numInputsBuilder + circuitsArray_Own[0] -> numInputsExecutor; i ++)
	{
		iOffset = numCircuits * (i - numInputsBuilder);

		value = circuitsArray_Own[0] -> gates[i] -> outputWire -> wirePermedValue;
		value = value ^ (circuitsArray_Own[0] -> gates[i] -> outputWire -> wirePerm & 0x01);


		for(j = 0; j < stat_SecParam; j ++)
		{
			tempWire = circuitsArray_Own[j] -> gates[i] -> outputWire;
			tempWire -> wireOutputKey = (unsigned char *) calloc(16, sizeof(unsigned char));

			if(0x00 == value)
			{
				memcpy(tempWire -> wireOutputKey, output[iOffset], 16);
				// memcpy(tempWire -> wireOutputKey, tempWire -> outputGarbleKeys -> key0, 16);
			}
			else
			{
				memcpy(tempWire -> wireOutputKey, output[iOffset], 16);
				// memcpy(tempWire -> wireOutputKey, tempWire -> outputGarbleKeys -> key1, 16);
			}
		}
	}

}


// NOTE : PartyID is 1 for P1, 0 for P2. For now you're just going to have to accept this and move on.
void run_HKE_2013_CnC_OT(int writeSocket, int readSocket, struct RawCircuit *rawInputCircuit, struct idAndValue *startOfInputChain, randctx *ctx, int partyID)
{
	struct Circuit **circuitsArray_Own, **circuitsArray_Partner;
	int i, commBufferLen = 0, J_setSize = 0;

	struct timespec ext_t_0, ext_t_1;
	struct timespec int_t_0, int_t_1;
	clock_t ext_c_0, ext_c_1;
	clock_t int_c_0, int_c_1;

	gmp_randstate_t *state;
	unsigned char *commBuffer, *J_set, *permedInputs, ***OT_Inputs, **OT_Outputs;
	struct eccParams *params;

	struct OT_NP_Receiver_Query **queries_Own;
	struct eccPoint ***NaorPinkasInputs, **queries_Partner, *C, *cTilde;
	struct builderInputCommitStruct *commitStruct, *partnersCommitStruct;
	mpz_t **aList;

	struct HKE_Output_Struct_Builder *outputStruct;
	struct DDH_Group *group;
	int numCircuits = 4, tempLength = 0, bufferOffset = 0;

	ub4 **circuitSeeds = (ub4 **) calloc(numCircuits, sizeof(ub4*));
	randctx **circuitCTXs = (randctx **) calloc(numCircuits, sizeof(randctx*));


	initRandGen();
	state = seedRandGen();
	group = get_128_Bit_Group(*state);
	params = initBrainpool_256_Curve();


	for(i = 0; i < numCircuits; i ++)
	{
		circuitCTXs[i] = (randctx*) calloc(1, sizeof(randctx));
		circuitSeeds[i] = getIsaacContext(circuitCTXs[i]);
	}

	struct idAndValue *startOfInputChainExec = readInputDetailsFile_Alt( (char*)"./inputs/adder_32bit.executor.input" );
	struct idAndValue *startOfInputChainBuilder = readInputDetailsFile_Alt( (char*)"./inputs/adder_32bit.builder.input" );


	ext_t_0 = timestamp();
	ext_c_0 = clock();

	printf("Partner has connected to us.\n");

	int_t_0 = timestamp();
	int_c_0 = clock();



	outputStruct = getOutputSecretsAndScheme(rawInputCircuit -> numOutputs, numCircuits, *state, group);
	C = setup_OT_NP_Sender(params, *state);
	cTilde = exchangeC_ForNaorPinkas(writeSocket, readSocket, C);

	aList = getNaorPinkasInputs(rawInputCircuit -> numInputs_P1, numCircuits, *state, params);
	NaorPinkasInputs = computeNaorPinkasInputs(cTilde, aList, rawInputCircuit -> numInputs_P1, numCircuits, params);

	circuitsArray_Own = buildAll_HKE_Circuits(rawInputCircuit, startOfInputChain, C, NaorPinkasInputs, outputStruct, params,
											ctx, circuitCTXs, circuitSeeds, numCircuits, partyID);
	circuitsArray_Partner = (struct Circuit **) calloc(numCircuits, sizeof(struct Circuit *));
	
	OT_Inputs = getAllInputKeys(circuitsArray_Own, numCircuits);
	permedInputs = getPermedInputValuesExecutor(circuitsArray_Own);


	for(i = 0; i < numCircuits; i++)
	{
		setCircuitsInputs_Hardcode(startOfInputChain, circuitsArray_Own[i], 0xFF);
		sendCircuit(writeSocket, readSocket, circuitsArray_Own[i]);
	}
	for(i = 0; i < stat_SecParam; i ++)
	{
		circuitsArray_Partner[i] = receiveFullCircuit(writeSocket, readSocket);
	}


	// Having received the circuits we now have the OT.
	// Note we do this after the circuits have been built and sent.
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

	// setInputsFromCharArray(circuitsArray_Own, output, stat_SecParam);

	setInputsFromNaorPinkas(circuitsArray_Partner, OT_Outputs, numCircuits);
	for(i = 0; i < numCircuits; i ++)
	{
		setCircuitsInputs_Hardcode(startOfInputChainExec, circuitsArray_Own[i], 0xFF);
		setCircuitsInputs_Hardcode(startOfInputChainBuilder, circuitsArray_Own[i], 0xFF);
		// runCircuitExec( circuitsArray_Own[i], 0, 0 );
		// printOutputHexString(circuitsArray_Own[i]);
		// runCircuitExec( circuitsArray_Partner[i], 0, 0 );
		// printOutputHexString(circuitsArray_Partner[i]);
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

	/*
	struct Circuit **circuitsArray_Own;
	int i, commBufferLen = 0, J_setSize = 0;

	struct timespec ext_t_0, ext_t_1;
	struct timespec int_t_0, int_t_1;
	clock_t ext_c_0, ext_c_1;
	clock_t int_c_0, int_c_1;

	gmp_randstate_t *state;
	unsigned char *commBuffer, *J_set, *permedInputs, ***OT_Inputs, **OT_Outputs;
	struct eccParams *params;

	struct OT_NP_Receiver_Query **queries_Own;
	struct eccPoint ***NaorPinkasInputs, *C, *cTilde, **queries_Partner;
	struct builderInputCommitStruct *commitStruct, *partnersCommitStruct;
	mpz_t **aList;

	struct HKE_Output_Struct_Builder *outputStruct;
	struct DDH_Group *group;
	int numCircuits = 4, tempLength = 0, bufferOffset = 0;

	ub4 **circuitSeeds = (ub4 **) calloc(numCircuits, sizeof(ub4*));
	randctx **circuitCTXs = (randctx **) calloc(numCircuits, sizeof(randctx*));


	initRandGen();
	state = seedRandGen();
	group = get_128_Bit_Group(*state);
	params = initBrainpool_256_Curve();


	for(i = 0; i < numCircuits; i ++)
	{
		circuitCTXs[i] = (randctx*) calloc(1, sizeof(randctx));
		circuitSeeds[i] = getIsaacContext(circuitCTXs[i]);
	}
	*/

	set_up_server_socket(destRead, readSocket, mainReadSock, readPort);
	set_up_server_socket(destWrite, writeSocket, mainWriteSock, writePort);


	/*
	struct idAndValue *startOfInputChainExec = readInputDetailsFile_Alt( (char*)"./inputs/adder_32bit.executor.input" );
	struct idAndValue *startOfInputChainBuilder = readInputDetailsFile_Alt( (char*)"./inputs/adder_32bit.builder.input" );


	ext_t_0 = timestamp();
	ext_c_0 = clock();

	printf("P2 has connected to us.\n");

	int_t_0 = timestamp();
	int_c_0 = clock();



	outputStruct = getOutputSecretsAndScheme(rawInputCircuit -> numOutputs, numCircuits, *state, group);
	C = setup_OT_NP_Sender(params, *state);
	cTilde = exchangeC_ForNaorPinkas(writeSocket, readSocket, C);

	aList = getNaorPinkasInputs(rawInputCircuit -> numInputs_P1, numCircuits, *state, params);
	NaorPinkasInputs = computeNaorPinkasInputs(cTilde, aList, rawInputCircuit -> numInputs_P1, numCircuits, params);

	circuitsArray_Own = buildAll_HKE_Circuits(rawInputCircuit, startOfInputChain, C, NaorPinkasInputs, outputStruct, params, ctx, circuitCTXs, circuitSeeds, numCircuits, 1);
	OT_Inputs = getAllInputKeys(circuitsArray_Own, numCircuits);
	permedInputs = getPermedInputValuesExecutor(circuitsArray_Own);


	queries_Own = NaorPinkas_OT_Produce_Queries(rawInputCircuit -> numInputs_P1, permedInputs, state, params, cTilde);
	queries_Partner = NaorPinkas_OT_Exchange_Queries(writeSocket, readSocket, rawInputCircuit -> numInputs_P1, queries_Own);

	NaorPinkas_OT_Sender_Transfer(writeSocket, readSocket, rawInputCircuit -> numInputs_P1, OT_Inputs, state, numCircuits, queries_Partner, params, C);
	OT_Outputs = NaorPinkas_OT_Receiver_Transfer(writeSocket, readSocket, rawInputCircuit -> numInputs_P1, permedInputs, state, numCircuits, queries_Own, params, cTilde);


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


	setInputsFromNaorPinkas(circuitsArray_Own, OT_Outputs, numCircuits);
	for(i = 0; i < numCircuits; i ++)
	{
		setCircuitsInputs_Hardcode(startOfInputChainExec, circuitsArray_Own[i], 0xFF);
		setCircuitsInputs_Hardcode(startOfInputChainBuilder, circuitsArray_Own[i], 0xFF);
		runCircuitExec( circuitsArray_Own[i], 0, 0 );
		printOutputHexString(circuitsArray_Own[i]);
	}

	*/

	run_HKE_2013_CnC_OT(writeSocket, readSocket, rawInputCircuit, startOfInputChain, ctx, 1);

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

	/*
	struct Circuit **circuitsArray_Own;
	int i, commBufferLen = 0, J_setSize = 0;

	struct timespec ext_t_0, ext_t_1;
	struct timespec int_t_0, int_t_1;
	clock_t ext_c_0, ext_c_1;
	clock_t int_c_0, int_c_1;

	gmp_randstate_t *state;
	unsigned char *commBuffer, *J_set, *permedInputs, ***OT_Inputs, **OT_Outputs;
	struct eccParams *params;

	struct OT_NP_Receiver_Query **queries_Own;
	struct eccPoint ***NaorPinkasInputs, **queries_Partner, *C, *cTilde;
	struct builderInputCommitStruct *commitStruct, *partnersCommitStruct;
	mpz_t **aList;

	struct HKE_Output_Struct_Builder *outputStruct;
	struct DDH_Group *group;
	int numCircuits = 4, tempLength = 0, bufferOffset = 0;

	ub4 **circuitSeeds = (ub4 **) calloc(numCircuits, sizeof(ub4*));
	randctx **circuitCTXs = (randctx **) calloc(numCircuits, sizeof(randctx*));


	initRandGen();
	state = seedRandGen();
	group = get_128_Bit_Group(*state);
	params = initBrainpool_256_Curve();


	for(i = 0; i < numCircuits; i ++)
	{
		circuitCTXs[i] = (randctx*) calloc(1, sizeof(randctx));
		circuitSeeds[i] = getIsaacContext(circuitCTXs[i]);
	}
	*/

	set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr_write);
	set_up_client_socket(readSocket, ipAddress, readPort, serv_addr_read);

	/*
	struct idAndValue *startOfInputChainExec = readInputDetailsFile_Alt( (char*)"./inputs/adder_32bit.executor.input" );
	struct idAndValue *startOfInputChainBuilder = readInputDetailsFile_Alt( (char*)"./inputs/adder_32bit.builder.input" );


	ext_t_0 = timestamp();
	ext_c_0 = clock();

	printf("P1 has connected to us.\n");

	int_t_0 = timestamp();
	int_c_0 = clock();



	outputStruct = getOutputSecretsAndScheme(rawInputCircuit -> numOutputs, numCircuits, *state, group);
	C = setup_OT_NP_Sender(params, *state);
	cTilde = exchangeC_ForNaorPinkas(writeSocket, readSocket, C);

	aList = getNaorPinkasInputs(rawInputCircuit -> numInputs_P1, numCircuits, *state, params);
	NaorPinkasInputs = computeNaorPinkasInputs(cTilde, aList, rawInputCircuit -> numInputs_P1, numCircuits, params);

	circuitsArray_Own = buildAll_HKE_Circuits(rawInputCircuit, startOfInputChain, C, NaorPinkasInputs, outputStruct, params, ctx, circuitCTXs, circuitSeeds, numCircuits, 0);
	OT_Inputs = getAllInputKeys(circuitsArray_Own, numCircuits);
	permedInputs = getPermedInputValuesExecutor(circuitsArray_Own);


	queries_Own = NaorPinkas_OT_Produce_Queries(rawInputCircuit -> numInputs_P1, permedInputs, state, params, cTilde);
	queries_Partner = NaorPinkas_OT_Exchange_Queries(writeSocket, readSocket, rawInputCircuit -> numInputs_P1, queries_Own);
	NaorPinkas_OT_Sender_Transfer(writeSocket, readSocket, rawInputCircuit -> numInputs_P1, OT_Inputs, state, numCircuits, queries_Partner, params, C);
	OT_Outputs = NaorPinkas_OT_Receiver_Transfer(writeSocket, readSocket, rawInputCircuit -> numInputs_P1, permedInputs, state, numCircuits, queries_Own, params, cTilde);


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

	// setInputsFromCharArray(circuitsArray_Own, output, stat_SecParam);

	setInputsFromNaorPinkas(circuitsArray_Own, OT_Outputs, numCircuits);
	for(i = 0; i < numCircuits; i ++)
	{
		setCircuitsInputs_Hardcode(startOfInputChainExec, circuitsArray_Own[i], 0xFF);
		setCircuitsInputs_Hardcode(startOfInputChainBuilder, circuitsArray_Own[i], 0xFF);
		runCircuitExec( circuitsArray_Own[i], 0, 0 );
		printOutputHexString(circuitsArray_Own[i]);
	}
	*/
	run_HKE_2013_CnC_OT(writeSocket, readSocket, rawInputCircuit, startOfInputChain, ctx, 0);


	close_client_socket(readSocket);
	close_client_socket(writeSocket);


	freeRawCircuit(rawInputCircuit);
	free_idAndValueChain(startOfInputChain);
}


