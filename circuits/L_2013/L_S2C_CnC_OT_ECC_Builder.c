void full_CnC_OT_Mod_Sender_ECC(int writeSocket, int readSocket, struct Circuit **circuitsArray, unsigned char **Xj_checkValues,
								gmp_randstate_t *state, int stat_SecParam, int comp_SecParam)
{
	struct params_CnC_ECC *params_S;

	struct CnC_OT_Mod_CTs **CTs;
	struct CnC_OT_Mod_Check_CT **checkCTs;

	struct tildeCRS *fullTildeCRS;
	struct tildeList *receivedTildeList;
	struct twoDH_Tuples **tuplesList;
	struct jSetCheckTildes *checkTildes;
	struct wire *tempWire;

	unsigned char *commBuffer;
	int i, j, iOffset = 0, numInputsBuilder;
	int commBufferLen, verified = 0, bufferOffset;


	params_S = setup_CnC_OT_Mod_Full_Sender(writeSocket, readSocket, *state);
	free(commBuffer);

	numInputsBuilder = circuitsArray[0] -> numInputsBuilder;

	CTs = (struct CnC_OT_Mod_CTs **) calloc(stat_SecParam, sizeof(struct CnC_OT_Mod_CTs *));


	bufferOffset = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	fullTildeCRS = deserialiseTildeCRS(commBuffer, circuitsArray[0] -> numInputsExecutor, stat_SecParam, &bufferOffset);
	free(commBuffer);
	
	tuplesList = getAllTuplesVerifier(writeSocket, readSocket, params_S, circuitsArray[0] -> numInputsExecutor, stat_SecParam, fullTildeCRS, state);
	verified = ZKPoK_Verifier_ECC_1Of2_Parallel(writeSocket, readSocket, circuitsArray[0] -> numInputsExecutor, params_S -> params, tuplesList, state);
	printf("%d\n", verified);

	iOffset = 0;
	for(i = numInputsBuilder; i < numInputsBuilder + circuitsArray[0] -> numInputsExecutor; i ++)
	{
		receivedTildeList = fullTildeCRS -> lists[iOffset];

		commBufferLen = 0;
		#pragma omp parallel for private(j, tempWire) schedule(auto)
		for(j = 0; j < stat_SecParam; j ++)
		{
			tempWire = circuitsArray[j] -> gates[i] -> outputWire;
			CTs[j] = transfer_CnC_OT_Mod_Enc_i_j(params_S, 16,
												receivedTildeList,
												tempWire -> outputGarbleKeys -> key0, tempWire -> outputGarbleKeys -> key1,
												*state, j);
		}
		commBuffer = serialise_Mod_CTs(CTs, stat_SecParam, &commBufferLen, 16);
		sendBoth(writeSocket, commBuffer, commBufferLen);
		free(commBuffer);

		for(j = 0; j < stat_SecParam; j ++)
		{
			clearECC_Point(CTs[j] -> u_0);
			clearECC_Point(CTs[j] -> u_1);
			free(CTs[j] -> w_0);
			free(CTs[j] -> w_1);
		}

		iOffset ++;
	}


	bufferOffset = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	checkTildes = deserialise_jSet_CheckTildes(commBuffer, &bufferOffset);
	free(commBuffer);


	commBufferLen = 0;
	checkCTs = checkCnC_OT_Mod_Enc(params_S, checkTildes, Xj_checkValues, *state);
	commBuffer = serialise_OT_Mod_Check_CTs(checkCTs, stat_SecParam, &commBufferLen, 16);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);


	ZKPoK_Ext_DH_TupleVerifierAll(writeSocket, readSocket, stat_SecParam,
								params_S -> params -> g, params_S -> crs -> g_1,
								checkTildes -> h_tildeList,
								params_S -> params, state);
}
