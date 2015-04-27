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



unsigned char *full_CnC_OT_Mod_Receiver_ECC(int writeSocket, int readSocket, struct Circuit **circuitsArray,
											gmp_randstate_t *state, struct idAndValue *startOfInputChain, unsigned char *permedInputs,
											int stat_SecParam, int comp_SecParam)
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
	int bufferOffset = 0, commBufferLen;

	struct timespec int_t_0, int_t_1;
	clock_t int_c_0, int_c_1;


	params_R = setup_CnC_OT_Mod_Full_Receiver(writeSocket, readSocket, stat_SecParam, comp_SecParam, *state);

	numInputsBuilder = circuitsArray[0] -> numInputsBuilder;
	mpz_init(r_i);
	CTs = (struct CnC_OT_Mod_CTs **) calloc(stat_SecParam, sizeof(struct CnC_OT_Mod_CTs *));


	fullTildeCRS = initTildeCRS(circuitsArray[0] -> numInputsExecutor, params_R -> params, *state);
	iOffset = 0;
	for(i = 0; i < circuitsArray[0] -> numInputsExecutor; i ++)
	{
		value = permedInputs[i];

		fullTildeCRS -> lists[i] = initTildeList(stat_SecParam, fullTildeCRS -> r_List[i], params_R -> crs, params_R -> params, value);
	}

	commBufferLen = 0;
	commBuffer = serialiseTildeCRS(fullTildeCRS, circuitsArray[0] -> numInputsExecutor, stat_SecParam, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);


	int_t_0 = timestamp();
	int_c_0 = clock();

	tuplesList = getAllTuplesProver(writeSocket, readSocket, params_R, circuitsArray[0] -> numInputsExecutor, stat_SecParam, fullTildeCRS, state);
	ZKPoK_Prover_ECC_1Of2_Parallel(writeSocket, readSocket, circuitsArray[0] -> numInputsExecutor, params_R -> params,
								tuplesList, fullTildeCRS -> r_List, startOfInputChain, state);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Parallel ZKPoK");



	for(i = 0; i < circuitsArray[0] -> numInputsExecutor; i ++)
	{
		value = permedInputs[i];

		bufferOffset = 0;
		commBuffer = receiveBoth(readSocket, commBufferLen);
		CTs = deserialise_Mod_CTs(commBuffer, &bufferOffset, 16);
		free(commBuffer);


		#pragma omp parallel for private(j, tempWire) schedule(auto)
		for(j = 0; j < stat_SecParam; j ++)
		{
			tempWire = circuitsArray[j] -> gates[i + numInputsBuilder] -> outputWire;

			if(0x00 == value)
			{
				tempWire -> outputGarbleKeys -> key0 = output_CnC_OT_Mod_Dec_i_j(params_R, fullTildeCRS -> r_List[i], CTs[j], 16, value, j);
				tempWire -> outputGarbleKeys -> key1 = output_CnC_OT_Mod_Dec_i_j_Alt(params_R, fullTildeCRS -> r_List[i], CTs[j], 16, value, j);

				memcpy(tempWire -> wireOutputKey, tempWire -> outputGarbleKeys -> key0, 16);
			}
			else
			{
				tempWire -> outputGarbleKeys -> key0 = output_CnC_OT_Mod_Dec_i_j_Alt(params_R, fullTildeCRS -> r_List[i], CTs[j], 16, value, j);
				tempWire -> outputGarbleKeys -> key1 = output_CnC_OT_Mod_Dec_i_j(params_R, fullTildeCRS -> r_List[i], CTs[j], 16, value, j);

				memcpy(tempWire -> wireOutputKey, tempWire -> outputGarbleKeys -> key1, 16);
			}
		}

		for(j = 0; j < stat_SecParam; j ++)
		{
			clearECC_Point(CTs[j] -> u_0);
			clearECC_Point(CTs[j] -> u_1);
			free(CTs[j] -> w_0);
			free(CTs[j] -> w_1);
		}
	}



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





unsigned char *full_CnC_OT_Mod_Receiver_ECC_Alt(int writeSocket, int readSocket, unsigned char ***OT_Outputs, int numInputsExecutor,
											gmp_randstate_t *state, struct idAndValue *startOfInputChain, unsigned char *permedInputs,
											int stat_SecParam, int comp_SecParam)
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
	int bufferLength = 0, i, j, iOffset = 0;
	int bufferOffset = 0, commBufferLen, u_v_index;

	struct timespec int_t_0, int_t_1;
	clock_t int_c_0, int_c_1;


	params_R = setup_CnC_OT_Mod_Full_Receiver(writeSocket, readSocket, stat_SecParam, comp_SecParam, *state);

	mpz_init(r_i);
	CTs = (struct CnC_OT_Mod_CTs **) calloc(stat_SecParam, sizeof(struct CnC_OT_Mod_CTs *));


	fullTildeCRS = initTildeCRS(numInputsExecutor, params_R -> params, *state);
	for(i = 0; i < numInputsExecutor; i ++)
	{
		value = permedInputs[i];

		fullTildeCRS -> lists[i] = initTildeList(stat_SecParam, fullTildeCRS -> r_List[i], params_R -> crs, params_R -> params, value);
	}

	commBufferLen = 0;
	commBuffer = serialiseTildeCRS(fullTildeCRS, numInputsExecutor, stat_SecParam, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);


	int_t_0 = timestamp();
	int_c_0 = clock();

	tuplesList = getAllTuplesProver(writeSocket, readSocket, params_R, numInputsExecutor, stat_SecParam, fullTildeCRS, state);
	ZKPoK_Prover_ECC_1Of2_Parallel(writeSocket, readSocket, numInputsExecutor, params_R -> params,
								tuplesList, fullTildeCRS -> r_List, startOfInputChain, state);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Parallel ZKPoK");


	(*OT_Outputs) = (unsigned char **) calloc(2 * numInputsExecutor * stat_SecParam, sizeof(unsigned char *));

	for(i = 0; i < numInputsExecutor; i ++)
	{
		value = permedInputs[i];


		bufferOffset = 0;
		commBuffer = receiveBoth(readSocket, commBufferLen);
		CTs = deserialise_Mod_CTs(commBuffer, &bufferOffset, 16);
		free(commBuffer);

		#pragma omp parallel for private(j, iOffset, u_v_index) schedule(auto)
		for(j = 0; j < stat_SecParam; j ++)
		{
			iOffset = stat_SecParam * i;
			u_v_index = (2 * iOffset) + (2 * j);

			if(0x00 == value)
			{
				(*OT_Outputs)[u_v_index + 0] = output_CnC_OT_Mod_Dec_i_j(params_R, fullTildeCRS -> r_List[i], CTs[j], 16, value, j);
				(*OT_Outputs)[u_v_index + 1] = output_CnC_OT_Mod_Dec_i_j_Alt(params_R, fullTildeCRS -> r_List[i], CTs[j], 16, value, j);
			}
			else
			{
				(*OT_Outputs)[u_v_index + 0] = output_CnC_OT_Mod_Dec_i_j_Alt(params_R, fullTildeCRS -> r_List[i], CTs[j], 16, value, j);
				(*OT_Outputs)[u_v_index + 1] = output_CnC_OT_Mod_Dec_i_j(params_R, fullTildeCRS -> r_List[i], CTs[j], 16, value, j);
			}
		}

		for(j = 0; j < stat_SecParam; j ++)
		{
			clearECC_Point(CTs[j] -> u_0);
			clearECC_Point(CTs[j] -> u_1);
			free(CTs[j] -> w_0);
			free(CTs[j] -> w_1);
		}
	}



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



int verifyB_Lists(unsigned char ***hashedB_List, unsigned char ***b_List, int numInputsExecutor)
{
	unsigned char *hashedB0, *hashedB1;
	unsigned char *delta_0 = (unsigned char *) calloc(16, sizeof(unsigned char));
	unsigned char *delta_I = (unsigned char *) calloc(16, sizeof(unsigned char));
	int i, k, verifiedAll = 0;

	// Get the first XOR, we're checking to see that all the XORs are the same so
	// we just grab the first one and check all equal to this one.
	for(k = 0; k < 16; k ++)
	{
		delta_0[k] = b_List[0][i][k] ^ b_List[1][i][k];
	}

	for(i = 0; i < numInputsExecutor; i ++)
	{
		// Check the bLists are consistent with the Hashes we were sent earlier
		hashedB0 = sha256_full(b_List[0][i], 16);
		hashedB1 = sha256_full(b_List[1][i], 16);

		verifiedAll |= memcmp(hashedB_List[0][i], hashedB0, 16);
		verifiedAll |= memcmp(hashedB_List[1][i], hashedB1, 16);

		// Check that 
		for(k = 0; k < 16; k ++)
		{
			delta_I[k] = b_List[0][i][k] ^ b_List[1][i][k];
		}
		verifiedAll |= memcmp(delta_0, delta_I, 16);

		free(hashedB0);
		free(hashedB1);
	}

	return verifiedAll;
}



int secretInputsToCheckCircuitsConsistentOutputs(struct Circuit **circuitsArray, struct RawCircuit *rawInputCircuit,
								struct eccPoint ***consistentInputs, ub4 **circuitSeeds,
								unsigned char **b0List, unsigned char **b1List, struct eccParams *params,
								unsigned char *J_set, int J_setSize, int stat_SecParam)
{
	struct Circuit *tempGarbleCircuit;
	struct wire *tempWire;
	//struct eccPoint ***consistentInputs;

	int i, j, temp = 0, k = 0, *idList = (int*) calloc(J_setSize, sizeof(int));
	randctx **tempCTX = (randctx **) calloc(J_setSize, sizeof(randctx*));


	for(j = 0; j < stat_SecParam; j ++)
	{
		if(0x01 == J_set[j])
		{
			tempCTX[k] = (randctx*) calloc(1, sizeof(randctx));
			setIsaacContextFromSeed(tempCTX[k], circuitSeeds[j]);
			idList[k] = j;
			k ++;
		}
	}

	//consistentInputs = getAllConsistentInputsAsPoints(secret_J_set, public_inputs -> public_keyPairs, params, J_set, stat_SecParam, rawInputCircuit -> numInputs_P1);


	#pragma omp parallel for default(shared) private(i, j, k, tempWire, tempGarbleCircuit) reduction(|:temp) 
	for(j = 0; j < J_setSize; j ++)
	{
		k = idList[j];
		for(i = 0; i < rawInputCircuit -> numInputs_P1; i ++)
		{
			tempWire = circuitsArray[k] -> gates[i] -> outputWire;
			tempWire -> outputGarbleKeys = (struct bitsGarbleKeys*) calloc(1, sizeof(struct bitsGarbleKeys));

			tempWire -> outputGarbleKeys -> key0 = hashECC_Point(consistentInputs[k][2 * i], 16);
			tempWire -> outputGarbleKeys -> key1 = hashECC_Point(consistentInputs[k][2 * i + 1], 16);
		}

		tempGarbleCircuit = readInCircuit_FromRaw_Seeded_ConsistentInputOutput(tempCTX[j], rawInputCircuit, consistentInputs[k], b0List, b1List, k, params);
			
		temp |= compareCircuit(rawInputCircuit, circuitsArray[k], tempGarbleCircuit);

		freeTempGarbleCircuit(tempGarbleCircuit);
	}

	return temp;
}


int proveConsistencyEvaluationKeys_Exec_L_2013(int writeSocket, int readSocket,
											unsigned char *J_set, int J_setSize,
											struct eccPoint **builderInputs,
											struct public_builderPRS_Keys *public_inputs,
											struct eccParams *params,
											struct secCompExecutorOutput *secComp, gmp_randstate_t *state)
{
	struct eccPoint **concatBuildersInputs, **concatPublicCircuitKeys;
	unsigned char *concat_J_set;
	int totalNumInputs, totalBuildersInputs, totalNumCircuitKeys, totalJ_setSize;
	int i, j, k, k1, k2, consistency = 0;


	totalNumInputs = public_inputs -> numKeyPairs;
	totalNumCircuitKeys = public_inputs -> stat_SecParam + secComp -> pubInputGroup -> public_inputs -> stat_SecParam;
	totalJ_setSize = J_setSize + secComp -> J_setSize;
	totalBuildersInputs = (totalNumCircuitKeys - totalJ_setSize) * totalNumInputs;


	concatBuildersInputs = (struct eccPoint **) calloc(totalBuildersInputs, sizeof(struct eccPoint *));
	concatPublicCircuitKeys = (struct eccPoint **) calloc(totalNumCircuitKeys, sizeof(struct eccPoint *));
	concat_J_set = (unsigned char *) calloc(totalNumCircuitKeys, sizeof(unsigned char));


	k = 0;
	k1 = 0;
	k2 = 0;
	for(i = 0; i < public_inputs -> numKeyPairs; i ++)
	{
		for(j = 0; j < public_inputs -> stat_SecParam - J_setSize; j ++)
		{
			concatBuildersInputs[k ++] = builderInputs[k1 ++];
		}
		for(j = 0; j < secComp -> pubInputGroup -> public_inputs -> stat_SecParam - secComp -> J_setSize; j ++)
		{
			concatBuildersInputs[k ++] = secComp -> builderInputs[k2 ++];
		}
	}

	j = 0;
	for(i = 0; i < public_inputs -> stat_SecParam; i ++)
	{
		concatPublicCircuitKeys[i] = public_inputs -> public_circuitKeys[i];
	}
	for(; i < totalNumCircuitKeys; i ++)
	{
		concatPublicCircuitKeys[i] = secComp -> pubInputGroup -> public_inputs -> public_circuitKeys[j ++];
	}


	memcpy(concat_J_set, J_set, public_inputs -> stat_SecParam);
	memcpy(concat_J_set + public_inputs -> stat_SecParam, secComp -> J_set, secComp -> pubInputGroup -> public_inputs -> stat_SecParam);


	consistency = proveConsistencyEvaluationKeys_Exec(writeSocket, readSocket, concat_J_set, totalJ_setSize,
									concatBuildersInputs, public_inputs -> public_keyPairs, concatPublicCircuitKeys,
									public_inputs -> numKeyPairs, totalNumCircuitKeys, params, state);

	printf("\nConsistency check = %d\n", consistency);

	return consistency;
}
