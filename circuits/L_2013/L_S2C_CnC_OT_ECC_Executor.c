void clearPublicInputsWithGroup(struct publicInputsWithGroup *pubInputGroup)
{
	int i;

	#pragma omp parallel for private(i) schedule(auto)
	for(i = 0; i < pubInputGroup -> public_inputs -> numKeyPairs; i ++)
	{
		clearECC_Point(pubInputGroup -> public_inputs -> public_keyPairs[i][0]);
		clearECC_Point(pubInputGroup -> public_inputs -> public_keyPairs[i][1]);

		free(pubInputGroup -> public_inputs -> public_keyPairs[i]);
	}
	#pragma omp parallel for private(i) schedule(auto)
	for(i = 0; i < pubInputGroup -> public_inputs -> stat_SecParam; i ++)
	{
		clearECC_Point(pubInputGroup -> public_inputs -> public_circuitKeys[i]);
	}
	free(pubInputGroup -> public_inputs -> public_circuitKeys);
	freeECC_Params(pubInputGroup -> params);
}



unsigned char *full_CnC_OT_Mod_Receiver_ECC(int writeSocket, int readSocket, struct Circuit **circuitsArray, gmp_randstate_t *state,
						struct idAndValue *startOfInputChain, int stat_SecParam, int comp_SecParam)
{
	struct params_CnC_ECC *params_R;

	struct tildeCRS *fullTildeCRS;
	struct tildeList *testTildeList;
	struct twoDH_Tuples **tuplesList;
	struct jSetCheckTildes *checkTildes;
	struct CnC_OT_Mod_CTs **CTs;
	struct CnC_OT_Mod_Check_CT **checkCTs;
	mpz_t r_i;

	struct wire *tempWire;

	unsigned char *commBuffer, value;
	int bufferLength = 0, i, j, iOffset = 0, numInputsBuilder;
	int bufferOffset = 0, commBufferLen, CT_Index;

	struct timespec int_t_0, int_t_1;
	clock_t int_c_0, int_c_1;


	params_R = setup_CnC_OT_Mod_Full_Receiver(writeSocket, readSocket, stat_SecParam, comp_SecParam, *state);

	numInputsBuilder = circuitsArray[0] -> numInputsBuilder;
	mpz_init(r_i);
	CTs = (struct CnC_OT_Mod_CTs **) calloc(stat_SecParam, sizeof(struct CnC_OT_Mod_CTs *));


	fullTildeCRS = initTildeCRS(circuitsArray[0] -> numInputsExecutor, params_R -> params, *state);
	iOffset = 0;
	for(i = numInputsBuilder; i < numInputsBuilder + circuitsArray[0] -> numInputsExecutor; i ++)
	{
		value = circuitsArray[0] -> gates[i] -> outputWire -> wirePermedValue;
		value = value ^ (circuitsArray[0] -> gates[i] -> outputWire -> wirePerm & 0x01);

		fullTildeCRS -> lists[iOffset] = initTildeList(stat_SecParam, fullTildeCRS -> r_List[iOffset], params_R -> crs, params_R -> params, value);
		iOffset ++;
	}

	commBufferLen = 0;
	commBuffer = serialiseTildeCRS(fullTildeCRS, circuitsArray[0] -> numInputsExecutor, stat_SecParam, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);


	int_t_0 = timestamp();
	int_c_0 = clock();

	// tuplesList = getAllTuplesProver(writeSocket, readSocket, params_R, circuitsArray[0] -> numInputsExecutor, stat_SecParam, fullTildeCRS, state);
	// ZKPoK_Prover_ECC_1Of2_Parallel(writeSocket, readSocket, circuitsArray[0] -> numInputsExecutor, params_R -> params,
	// 							tuplesList, fullTildeCRS -> r_List, startOfInputChain, state);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "\nParallel ZKPoK");

	bufferOffset = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	CTs = deserialise_Mod_CTs(commBuffer, &bufferOffset, 16);
	free(commBuffer);


	#pragma omp parallel for private(i, j, tempWire, iOffset, value, CT_Index) schedule(auto)
	for(i = numInputsBuilder; i < numInputsBuilder + circuitsArray[0] -> numInputsExecutor; i ++)
	{
		iOffset = i - numInputsBuilder;

		value = circuitsArray[0] -> gates[i] -> outputWire -> wirePermedValue;
		value = value ^ (circuitsArray[0] -> gates[i] -> outputWire -> wirePerm & 0x01);

		for(j = 0; j < stat_SecParam; j ++)
		{
			CT_Index = iOffset * stat_SecParam + j;
			tempWire = circuitsArray[j] -> gates[i] -> outputWire;
			if(0x00 == value)
			{
				tempWire -> outputGarbleKeys -> key0 = output_CnC_OT_Mod_Dec_i_j(params_R, fullTildeCRS -> r_List[iOffset], CTs[CT_Index], 16, value, j);
				tempWire -> outputGarbleKeys -> key1 = output_CnC_OT_Mod_Dec_i_j_Alt(params_R, fullTildeCRS -> r_List[iOffset], CTs[CT_Index], 16, value, j);

				memcpy(tempWire -> wireOutputKey, tempWire -> outputGarbleKeys -> key0, 16);
			}
			else
			{
				tempWire -> outputGarbleKeys -> key0 = output_CnC_OT_Mod_Dec_i_j_Alt(params_R, fullTildeCRS -> r_List[iOffset], CTs[CT_Index], 16, value, j);
				tempWire -> outputGarbleKeys -> key1 = output_CnC_OT_Mod_Dec_i_j(params_R, fullTildeCRS -> r_List[iOffset], CTs[CT_Index], 16, value, j);

				memcpy(tempWire -> wireOutputKey, tempWire -> outputGarbleKeys -> key1, 16);
			}
		}
	}

	/*
	for(j = 0; j < stat_SecParam; j ++)
	{
		clearECC_Point(CTs[j] -> u_0);
		clearECC_Point(CTs[j] -> u_1);
		free(CTs[j] -> w_0);
		free(CTs[j] -> w_1);
	}
	*/

	commBufferLen = 0;
	checkTildes = transfer_CheckValues_CnC_OT_Mod_Receiver(params_R, *state);
	commBuffer = serialise_jSet_CheckTildes(checkTildes, params_R -> crs -> stat_SecParam, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);

	bufferOffset = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	checkCTs = deserialise_OT_Mod_Check_CTs(commBuffer, &bufferOffset, 16);
	free(commBuffer);

	ZKPoK_Ext_DH_TupleProverAll(writeSocket, readSocket, stat_SecParam, checkTildes -> roe_jList, params_R -> crs -> alphas_List,
								params_R -> params -> g, params_R -> crs -> g_1,
								checkTildes -> h_tildeList,
								params_R -> params, state);


	return params_R -> crs -> J_set;
}




int secretInputsToCheckCircuitsConsistentOutputs(struct Circuit **circuitsArray, struct RawCircuit *rawInputCircuit,
								struct public_builderPRS_Keys *public_inputs, mpz_t *secret_J_set, unsigned int *seedList,
								unsigned char **b0List, unsigned char **b1List, struct eccParams *params,
								unsigned char *J_set, int J_setSize, int stat_SecParam)
{
	struct Circuit *tempGarbleCircuit;
	struct wire *tempWire;
	int i, j, temp = 0;


	for(j = 0; j < stat_SecParam; j ++)
	{
		if(0x01 == J_set[j])
		{
			// printf("Checkpoint Beta 1 => %d\n", j);
			fflush(stdout);

			for(i = 0; i < rawInputCircuit -> numInputsBuilder; i ++)
			{
				tempWire = circuitsArray[j] -> gates[i] -> outputWire;
				tempWire -> outputGarbleKeys = (struct bitsGarbleKeys*) calloc(1, sizeof(struct bitsGarbleKeys));

				tempWire -> outputGarbleKeys -> key0 = compute_Key_b_Input_i_Circuit_j(secret_J_set[j], public_inputs, params, i, 0x00);
				tempWire -> outputGarbleKeys -> key1 = compute_Key_b_Input_i_Circuit_j(secret_J_set[j], public_inputs, params, i, 0x01);
			}

			tempGarbleCircuit = readInCircuit_FromRaw_Seeded_ConsistentInputOutput(rawInputCircuit, seedList[j], secret_J_set[j], b0List, b1List, public_inputs, j, params);
			temp = compareCircuit(rawInputCircuit, circuitsArray[j], tempGarbleCircuit);

			// printf("Checkpoint Beta 3 => %d\n", j);
			fflush(stdout);
			freeTempGarbleCircuit(tempGarbleCircuit);
		}
	}

	return 1;
}