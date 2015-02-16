void builderInputGarbledKeys(struct Circuit *inputCircuit, struct secret_builderPRS_Keys *secret_inputs, struct public_builderPRS_Keys *public_inputs,
							struct DDH_Group *group, int j, unsigned char *R)
{
	struct bitsGarbleKeys *tempOutput;
	struct wire *tempWire;
	unsigned char inputBit;
	int i, gateID;


	printf("$$$$$\n");
	fflush(stdout);
	for(i = 0; i < inputCircuit -> numInputsBuilder; i ++)
	{
		gateID = inputCircuit -> execOrder[i];

		tempWire = inputCircuit -> gates[gateID] -> outputWire;

		if(0xFF == tempWire -> wireOwner)
		{
			printf("<%03d><%d> ", i, j);
			fflush(stdout);
			// tempWire -> outputGarbleKeys = genFreeXORPairInput(tempWire -> wirePerm, R);
			tempOutput = (struct bitsGarbleKeys*) calloc(1, sizeof(struct bitsGarbleKeys));
			printf(" + ");
			fflush(stdout);
			tempOutput -> key0 = compute_Key_b_Input_i_Circuit_j(secret_inputs, public_inputs, group, i, j, 0x00);
			printf(" + ");
			fflush(stdout);
			tempOutput -> key1 = compute_Key_b_Input_i_Circuit_j(secret_inputs, public_inputs, group, i, j, 0x01);
			printf(" +\n");
			fflush(stdout);

			free(tempOutput);
		}
	}
}



void full_CnC_OT_Sender(int writeSocket, int readSocket, struct Circuit **circuitsArray, gmp_randstate_t *state,
						int stat_SecParam, int comp_SecParam)
{
	struct params_CnC *params_S;
	struct otKeyPair **keyPairs_S;
	struct u_v_Pair **c_i_Array_S;

	struct wire *tempWire;

	unsigned char *commBuffer;
	int i, j, bufferLength = 0, iOffset = 0, numInputsBuilder;
	int totalOTs, u_v_index;

	int k;


	commBuffer = receiveBoth(readSocket, bufferLength);

	params_S = setup_CnC_OT_Sender(commBuffer);
	free(commBuffer);

	totalOTs = params_S -> crs -> stat_SecParam * circuitsArray[0] -> numInputsExecutor;
	numInputsBuilder = circuitsArray[0] -> numInputsBuilder;


	// When doing this properly the ZKPOK goes here.


	commBuffer = receiveBoth(readSocket, bufferLength);
	keyPairs_S = deserialise_PKs_otKeyPair_Array(commBuffer, totalOTs);
	free(commBuffer);


	c_i_Array_S = (struct u_v_Pair **) calloc(2*totalOTs, sizeof(struct u_v_Pair*));


	#pragma omp parallel for private(i, j, iOffset, u_v_index, tempWire)
	for(i = numInputsBuilder; i < numInputsBuilder + circuitsArray[0] -> numInputsExecutor; i ++)
	{
		iOffset = stat_SecParam * (i - numInputsBuilder);
		u_v_index = 2 * iOffset;

		for(j = 0; j < stat_SecParam; j ++)
		{
			tempWire = circuitsArray[j] -> gates[i] -> outputWire;

			CnC_OT_Transfer_One_Sender(tempWire -> outputGarbleKeys -> key0, tempWire -> outputGarbleKeys -> key1, 16,
										params_S, state, keyPairs_S[iOffset + j], c_i_Array_S, u_v_index, j);
			u_v_index += 2;
		}
	}

	bufferLength = 0;
	commBuffer = serialise_U_V_Pair_Array(c_i_Array_S, totalOTs * 2, &bufferLength);
	sendBoth(writeSocket, commBuffer, bufferLength);
	free(commBuffer);
}



struct Circuit **buildAllCircuits(char *circuitFilepath, char *inputFilepath, gmp_randstate_t state, int stat_SecParam)
{
	struct Circuit **circuitsArray = (struct Circuit **) calloc(stat_SecParam, sizeof(struct Circuit*));
	struct DDH_Group *group = getSchnorrGroup(1024, state);
	struct secret_builderPRS_Keys *secret_inputs;
	struct public_builderPRS_Keys *public_inputs;
	struct idAndValue *startOfInputChain, *start;

	unsigned char *R = generateRandBytes(16, 17);

	int j;


	for(j = 0; j < stat_SecParam; j++)
	{
		circuitsArray[j] = readInCircuitRTL(circuitFilepath);
	}


	secret_inputs = generateSecrets(circuitsArray[0] -> numInputsBuilder, stat_SecParam, group, state);
	public_inputs = computePublicInputs(secret_inputs, group);


	/*
	for(j = 0; j < stat_SecParam; j++)
	{
		builderInputGarbledKeys(circuitsArray[j], secret_inputs, public_inputs, group, j, R);
		garbleOutputTables(circuitsArray[j]);
	}
	printf("<d><>\n");
	fflush(stdout);
	*/


	startOfInputChain = readInputDetailsFile_Alt(inputFilepath);
	for(j = 0; j < stat_SecParam; j++)
	{
		start = startOfInputChain;
		setCircuitsInputs_Hardcode(start, circuitsArray[j], 0xFF);
	}
	free_idAndValueChain(startOfInputChain);

	return circuitsArray;
}