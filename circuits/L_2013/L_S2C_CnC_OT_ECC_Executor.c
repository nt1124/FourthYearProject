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
	int bufferOffset = 0, commBufferLen;


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

	tuplesList = getAllTuplesProver(writeSocket, readSocket, params_R, circuitsArray[0] -> numInputsExecutor, stat_SecParam, fullTildeCRS, state);
	ZKPoK_Prover_ECC_1Of2_Parallel(writeSocket, readSocket, circuitsArray[0] -> numInputsExecutor, params_R -> params,
								tuplesList, fullTildeCRS -> r_List, startOfInputChain, state);

	iOffset = 0;
	for(i = numInputsBuilder; i < numInputsBuilder + circuitsArray[0] -> numInputsExecutor; i ++)
	{
		value = circuitsArray[0] -> gates[i] -> outputWire -> wirePermedValue;
		value = value ^ (circuitsArray[0] -> gates[i] -> outputWire -> wirePerm & 0x01);

		mpz_set(r_i, fullTildeCRS -> r_List[iOffset]);
		testTildeList = fullTildeCRS -> lists[iOffset ++];


		bufferOffset = 0;
		commBuffer = receiveBoth(readSocket, commBufferLen);
		CTs = deserialise_Mod_CTs(commBuffer, &bufferOffset, 16);
		free(commBuffer);


		// This pragma as usual doesn't make a big difference, but as stat_secParam increases it will.
		#pragma omp parallel for private(j, tempWire) schedule(auto)
		for(j = 0; j < stat_SecParam; j ++)
		{
			tempWire = circuitsArray[j] -> gates[i] -> outputWire;
			if(0x00 == value)
			{
				tempWire -> outputGarbleKeys -> key0 = output_CnC_OT_Mod_Dec_i_j(params_R, r_i, CTs[j], 16, value, j);
				tempWire -> outputGarbleKeys -> key1 = output_CnC_OT_Mod_Dec_i_j_Alt(params_R, r_i, CTs[j], 16, value, j);

				memcpy(tempWire -> wireOutputKey, tempWire -> outputGarbleKeys -> key0, 16);
			}
			else
			{
				tempWire -> outputGarbleKeys -> key0 = output_CnC_OT_Mod_Dec_i_j_Alt(params_R, r_i, CTs[j], 16, value, j);
				tempWire -> outputGarbleKeys -> key1 = output_CnC_OT_Mod_Dec_i_j(params_R, r_i, CTs[j], 16, value, j);

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

