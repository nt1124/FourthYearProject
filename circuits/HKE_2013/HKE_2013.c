/*
// Function that gets the OT inputs as an array from the circuits.
unsigned char ***getAllInputKeys(struct Circuit **circuitsArray, int numCircuits)
{
	unsigned char ***allKeys = (unsigned char ***) calloc(2, sizeof(unsigned char **));
	struct wire *tempWire;
	int i, j, k = 0, numKeysToStore;
	int numInputsBuilder, numInputsExecutor;


	numInputsBuilder = circuitsArray[0] -> numInputsBuilder;
	numInputsExecutor = circuitsArray[0] -> numInputsExecutor;
	numKeysToStore = circuitsArray[0] -> numInputsExecutor * numCircuits;

	allKeys[0] = (unsigned char **) calloc(numKeysToStore, sizeof(unsigned char *));
	allKeys[1] = (unsigned char **) calloc(numKeysToStore, sizeof(unsigned char *));

	for(i = numInputsBuilder; i < numInputsBuilder + numInputsExecutor; i ++)
	{
		for(j = 0; j < numCircuits; j ++)
		{
			tempWire = circuitsArray[j] -> gates[i] -> outputWire;
			allKeys[0][k] = tempWire -> outputGarbleKeys -> key0;
			allKeys[1][k] = tempWire -> outputGarbleKeys -> key1;

			k ++;
		}

	}

	return allKeys;
}


unsigned char *getPermedInputValuesExecutor(struct Circuit **circuitsArray)
{
	int i, outputIndex = 0, numInputsBuilder, numInputsExecutor;
	unsigned char *output, value;


	numInputsBuilder = circuitsArray[0] -> numInputsBuilder;
	numInputsExecutor = circuitsArray[0] -> numInputsExecutor;
	output = (unsigned char *) calloc(numInputsExecutor, sizeof(unsigned char));

	for(i = numInputsBuilder; i < numInputsBuilder + numInputsExecutor; i ++)
	{
		value = circuitsArray[0] -> gates[i] -> outputWire -> wirePermedValue;
		output[outputIndex++] = value ^ (circuitsArray[0] -> gates[i] -> outputWire -> wirePerm & 0x01);
	}


	return output;
}
*/

unsigned char *tempSerialiseInputs(unsigned char ***OT_Inputs, int numInputs, int numCircuits)
{
	unsigned char *output = (unsigned char *) calloc(16 * numInputs * numCircuits, sizeof(unsigned char));
	int i, j, index = 0, inputIndex = 0;

	for(i = 0; i < numInputs; i ++)
	{
		for(j = 0; j < numCircuits; j ++)
		{
			memcpy(output + index, OT_Inputs[0][inputIndex], 16);
			index += 16;
			memcpy(output + index, OT_Inputs[1][inputIndex], 16);
			index += 16;
			inputIndex ++;
		}
		printf("\n");
	}


	return output;
}

unsigned char ***tempDeserialiseInputs(unsigned char *serialiseInput, int numInputs, int numCircuits)
{
	unsigned char ***output = (unsigned char ***) calloc(2, sizeof(unsigned char**));
	int i, j, index = 0, inputIndex = 0;


	output[0] = (unsigned char **) calloc(numInputs * numCircuits, sizeof(unsigned char *));
	output[1] = (unsigned char **) calloc(numInputs * numCircuits, sizeof(unsigned char *));

	for(i = 0; i < numInputs; i ++)
	{
		for(j = 0; j < numCircuits; j ++)
		{
			output[0][inputIndex] = (unsigned char *) calloc(16, sizeof(unsigned char));
			output[1][inputIndex] = (unsigned char *) calloc(16, sizeof(unsigned char));

			memcpy(output[0][inputIndex], serialiseInput + index, 16);
			index += 16;
			memcpy(output[1][inputIndex], serialiseInput + index, 16);
			index += 16;
			inputIndex ++;
		}
	}

	return output;
}


void justCos(struct Circuit *circuitOwn, struct Circuit *circuitPartner,
			struct idAndValue *startOfInputChainBuilder, struct idAndValue *startOfInputChainExec)
{
	struct idAndValue *current;
	struct wire *outputWireOwn, *outputWirePartner;


	current = startOfInputChainExec -> next;
	while(NULL != current)
	{
		outputWireOwn = circuitOwn -> gates[current -> id] -> outputWire;
		outputWirePartner = circuitPartner -> gates[current -> id] -> outputWire;

		if( 0x01 == current -> value )
		{
			memcpy(outputWirePartner -> wireOutputKey, outputWireOwn -> outputGarbleKeys -> key1, 16);
			outputWirePartner -> wirePermedValue = 0x01 ^ (0x01 & outputWireOwn -> wirePerm);
		}
		else if( 0x00 == current -> value )
		{
			memcpy(outputWirePartner -> wireOutputKey, outputWireOwn -> outputGarbleKeys -> key0, 16);
			outputWirePartner -> wirePermedValue = (0x01 & outputWireOwn -> wirePerm);
		}

		current = current -> next;
	}


	current = startOfInputChainBuilder -> next;
	while(NULL != current)
	{
		outputWireOwn = circuitOwn -> gates[current -> id] -> outputWire;
		outputWirePartner = circuitPartner -> gates[current -> id] -> outputWire;

		if( 0x01 == current -> value )
		{
			memcpy(outputWirePartner -> wireOutputKey, outputWireOwn -> outputGarbleKeys -> key1, 16);
			outputWirePartner -> wirePermedValue = 0x01 ^ (0x01 & outputWireOwn -> wirePerm);
		}
		else if( 0x00 == current -> value )
		{
			memcpy(outputWirePartner -> wireOutputKey, outputWireOwn -> outputGarbleKeys -> key0, 16);
			outputWirePartner -> wirePermedValue = (0x01 & outputWireOwn -> wirePerm);
		}

		current = current -> next;
	}

}

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


void setInputsFromNaorPinkas(struct Circuit **circuitsArray_Own, unsigned char **output, int numCircuits, int partyID)
{
	int i, j, iOffset, gateIndex, numInputsBuilder, numInputsExecutor;
	struct wire *tempWire;
	unsigned char value;


	numInputsBuilder = circuitsArray_Own[0] -> numInputsBuilder;
	numInputsExecutor = circuitsArray_Own[0] -> numInputsExecutor;


	for(i = 0; i < numInputsExecutor; i ++)
	{
		iOffset = numCircuits * i;
		gateIndex = i + numInputsBuilder * partyID;
		// printf("%d  -  %d\n", gateIndex, partyID);


		for(j = 0; j < numCircuits; j ++)
		{
			tempWire = circuitsArray_Own[j] -> gates[gateIndex] -> outputWire;
			tempWire -> wireOutputKey = (unsigned char *) calloc(16, sizeof(unsigned char));
			memcpy(tempWire -> wireOutputKey, output[iOffset + j], 16);
		}
	}

}


// NOTE : PartyID is 1 for P1, 0 for P2. For now you're just going to have to accept this and move on.
void run_HKE_2013_CnC_OT(int writeSocket, int readSocket, struct RawCircuit *rawInputCircuit, struct idAndValue *startOfInputChain, randctx *ctx, int partyID)
{
	struct Circuit **circuitsArray_Own, **circuitsArray_Partner;
	int i, commBufferLen = 0, J_setSize = 0, j;

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
	int numCircuits = 1, tempLength = 0, bufferOffset = 0;

	ub4 **circuitSeeds = (ub4 **) calloc(numCircuits, sizeof(ub4*));
	randctx **circuitCTXs = (randctx **) calloc(numCircuits, sizeof(randctx*));


	initRandGen();
	state = seedRandGen();
	group = get_128_Bit_Group(*state);
	params = initBrainpool_256_Curve();


	for(i = 0; i < numCircuits; i ++)
	{
		circuitCTXs[i] = (randctx*) calloc(1, sizeof(randctx));
		// circuitSeeds[i] = getIsaacContext(circuitCTXs[i]);
		circuitSeeds[i] = getIsaacContextPartyID(circuitCTXs[i], partyID);
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

	commBuffer = serialiseCircuit(circuitsArray_Own[0], &commBufferLen);

	circuitsArray_Partner[0] = (struct Circuit *) calloc(1, sizeof(struct Circuit));
	circuitsArray_Partner[0] -> numGates = circuitsArray_Own[0] -> numGates;
	circuitsArray_Partner[0] -> numInputs = circuitsArray_Own[0] -> numInputs;
	circuitsArray_Partner[0] -> numOutputs = circuitsArray_Own[0] -> numOutputs;
	circuitsArray_Partner[0] -> numInputsBuilder = circuitsArray_Own[0] -> numInputsBuilder;
	circuitsArray_Partner[0] -> numInputsExecutor = circuitsArray_Own[0] -> numInputsExecutor;
	circuitsArray_Partner[0] -> securityParam = circuitsArray_Own[0] -> securityParam;
	circuitsArray_Partner[0] -> execOrder = circuitsArray_Own[0] -> execOrder;


	// Copy across the ExecOrder into the bufferToSend.

	circuitsArray_Partner[0] -> gates = deserialiseCircuit(commBuffer, rawInputCircuit -> numGates);


	justCos(circuitsArray_Own[0], circuitsArray_Partner[0], startOfInputChainBuilder, startOfInputChainExec);

	runCircuitExec( circuitsArray_Partner[0], 0, 0 );
	printOutputHexString(circuitsArray_Partner[0]);

	for(i = 0; i < 64; i ++)
	{
		if(0xFF == circuitsArray_Own[0] -> gates[i] -> outputWire -> wireOwner)
		{
			printf("%d - 0 - ", i);
			for(j = 0; j < 16; j ++)
			{
				printf("%02X", circuitsArray_Own[0] -> gates[i] -> outputWire -> outputGarbleKeys -> key0[j]);
			}
			printf("\n");
			printf("%d - 1 - ", i);
			for(j = 0; j < 16; j ++)
			{
				printf("%02X", circuitsArray_Own[0] -> gates[i] -> outputWire -> outputGarbleKeys -> key1[j]);
			}
			printf("\n\n");
		}
	}

	for(i = 0; i < 32; i ++)
	{
		// if(0x00 == circuitsArray_Partner[0] -> gates[i] -> outputWire -> wireOwner)
		{
			printf("%d - 0 - ", i);
			for(j = 0; j < 16; j ++)
			{
				printf("%02X", OT_Inputs[0][i][j]);
			}
			printf("\n");
			printf("%d - 1 - ", i);
			for(j = 0; j < 16; j ++)
			{
				printf("%02X", OT_Inputs[1][i][j]);
			}
			printf("\n\n");
		}
	}

	// setCircuitsInputs_Values(startOfInputChainBuilder, circuitsArray_Partner[0], 0x00);
	// setCircuitsInputs_Values(startOfInputChainExec, circuitsArray_Partner[0], 0x00);
	/*
	for(i = 0; i < numCircuits; i++)
	{
		// setCircuitsInputs_Hardcode(startOfInputChain, circuitsArray_Own[i], 0xFF);
		sendCircuit(writeSocket, readSocket, circuitsArray_Own[i]);
	}
	for(i = 0; i < numCircuits; i ++)
	{
		circuitsArray_Partner[i] = receiveFullCircuit(writeSocket, readSocket);
		setCircuitsInputs_Values(startOfInputChain, circuitsArray_Partner[i], 0x00);
	}
	*/

	// Having received the circuits we now have the OT.
	// Note we do this after the circuits have been built and sent.

	permedInputs = getPermedInputValuesExecutor(circuitsArray_Partner);
	queries_Own = NaorPinkas_OT_Produce_Queries(rawInputCircuit -> numInputs_P1, permedInputs, state, params, cTilde);
	queries_Partner = NaorPinkas_OT_Exchange_Queries(writeSocket, readSocket, rawInputCircuit -> numInputs_P1, queries_Own);
	NaorPinkas_OT_Sender_Transfer(writeSocket, readSocket, rawInputCircuit -> numInputs_P1, OT_Inputs, state, numCircuits, queries_Partner, params, C);
	OT_Outputs = NaorPinkas_OT_Receiver_Transfer(writeSocket, readSocket, rawInputCircuit -> numInputs_P1, permedInputs, state, numCircuits, queries_Own, params, cTilde);

	for(i = 0; i < 32; i ++)
	{
		// if(0x00 == circuitsArray_Partner[0] -> gates[i] -> outputWire -> wireOwner)
		{
			printf("%d - ", i);
			for(j = 0; j < 16; j ++)
			{
				printf("%02X", OT_Outputs[i][j]);
			}
			printf("\n");
		}
	}
	/*
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


	// setInputsFromNaorPinkas(circuitsArray_Partner, OT_Outputs, numCircuits, partyID);

	for(i = 0; i < numCircuits; i ++)
	{
		// setCircuitsInputs_Hardcode(startOfInputChainExec, circuitsArray_Own[i], 0xFF);
		// setCircuitsInputs_Hardcode(startOfInputChainBuilder, circuitsArray_Own[i], 0xFF);

		for(j = 0; j < 16; j ++)
		{
			printf("%02X", circuitsArray_Own[i] -> gates[0] -> outputWire -> wireOutputKey[j]);
		}
		printf("\n");


		// runCircuitExec( circuitsArray_Own[i], 0, 0 );
		// printOutputHexString(circuitsArray_Own[i]);
		// runCircuitExec( circuitsArray_Partner[i], 0, 0 );
		// printOutputHexString(circuitsArray_Partner[i]);
	}

	printf("\n");

	for(i = 0; i < numCircuits; i ++)
	{
		for(j = 0; j < 16; j ++)
		{
			printf("%02X", circuitsArray_Partner[i] -> gates[0] -> outputWire -> wireOutputKey[j]);
		}
		printf("\n");
		freeCircuitStruct(circuitsArray_Own[i], 0);
		freeCircuitStruct(circuitsArray_Partner[i], 0);
	}
	*/
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


	// freeRawCircuit(rawInputCircuit);
	// free_idAndValueChain(startOfInputChain);
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


	// freeRawCircuit(rawInputCircuit);
	// free_idAndValueChain(startOfInputChain);
}


