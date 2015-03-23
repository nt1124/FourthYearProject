struct Circuit **buildAllCircuitsConsistentOutput(struct RawCircuit *rawInputCircuit, struct idAndValue *startOfInputChain,
												gmp_randstate_t state, int stat_SecParam, unsigned int *seedList,
												unsigned char **b0List, unsigned char **b1List,
												struct eccParams *params, struct secret_builderPRS_Keys *secret_inputs,
												struct public_builderPRS_Keys *public_inputs)
{
	struct Circuit **circuitsArray = (struct Circuit **) calloc(stat_SecParam, sizeof(struct Circuit*));

	struct idAndValue *start;

	unsigned char *R = generateRandBytes(16, 17);
	int j;


	for(j = 0; j < stat_SecParam; j++)
	{
		circuitsArray[j] = readInCircuit_FromRaw_Seeded_ConsistentInputOutput(rawInputCircuit, seedList[j], secret_inputs -> secret_circuitKeys[j],
																			b0List, b1List, public_inputs, j, params);
	}


	for(j = 0; j < stat_SecParam; j++)
	{
		start = startOfInputChain;
		setCircuitsInputs_Hardcode(start, circuitsArray[j], 0xFF);
	}

	
	free(R);

	return circuitsArray;
}


unsigned char ***generateConsistentOutputs(unsigned char *delta, int numInputs)
{
	unsigned char ***output = (unsigned char ***) calloc(2, sizeof(unsigned char **));
	int i, j;

	output[0] = (unsigned char **) calloc(numInputs, sizeof(unsigned char *));
	output[1] = (unsigned char **) calloc(numInputs, sizeof(unsigned char *));

	for(i = 0; i < numInputs; i ++)
	{
		output[0][i] = generateRandBytes(16, 16);
		output[1][i] = (unsigned char *) calloc(16, sizeof(unsigned char));

		for(j = 0; j < 16; j ++)
		{
			output[1][i][j] = output[0][i][j] ^ delta[j];
		}
	}

	return output;
}


unsigned char ***generateConsistentOutputsHashTables(unsigned char ***outputs, int numInputs)
{
	unsigned char ***hashedValues = (unsigned char ***) calloc(2, sizeof(unsigned char **));
	int i, j;

	hashedValues[0] = (unsigned char **) calloc(numInputs, sizeof(unsigned char *));
	hashedValues[1] = (unsigned char **) calloc(numInputs, sizeof(unsigned char *));

	// For each output value we hash it and store, this is then forwarded to the Executor
	// Who can then use it to determine the true value of an output gate.
	for(i = 0; i < numInputs; i ++)
	{
		hashedValues[0][i] = sha256_full(outputs[0][i], 16);
		hashedValues[1][i] = sha256_full(outputs[1][i], 16);
	}

	return hashedValues;
}



unsigned char *serialise3D_UChar_Array(unsigned char ***toSerialise, int numInputs, int hashOutputLengths, int *outputLength)
{
	unsigned char *outputBuffer = (unsigned char *) calloc(2 * numInputs * hashOutputLengths, sizeof(unsigned char));
	int i, outputOffset = 0;

	for(i = 0; i < numInputs; i ++)
	{
		memcpy(outputBuffer + outputOffset, toSerialise[0][i], hashOutputLengths);
		outputOffset += hashOutputLengths;

		memcpy(outputBuffer + outputOffset, toSerialise[1][i], hashOutputLengths);
		outputOffset += hashOutputLengths;
	}


	*outputLength = outputOffset;

	return outputBuffer;
}


unsigned char ***deserialise3D_UChar_Array(unsigned char *commBuffer, int numInputs, int hashOutputLengths, int *inputOffset)
{
	unsigned char ***output = (unsigned char ***) calloc(2, sizeof(unsigned char **));
	int i, offset = *inputOffset;


	output[0] = (unsigned char **) calloc(numInputs, sizeof(unsigned char *));
	output[1] = (unsigned char **) calloc(numInputs, sizeof(unsigned char *));

	for(i = 0; i < numInputs; i ++)
	{
		output[0][i] = (unsigned char *) calloc(hashOutputLengths, sizeof(unsigned char));
		memcpy(output[0][i], commBuffer + offset, hashOutputLengths);
		offset += hashOutputLengths;

		output[1][i] = (unsigned char *) calloc(hashOutputLengths, sizeof(unsigned char));
		memcpy(output[1][i], commBuffer + offset, hashOutputLengths);
		offset += hashOutputLengths;
	}


	*inputOffset = offset;

	return output;
}



void full_CnC_OT_Mod_Sender_ECC(int writeSocket, int readSocket, int numInputsExecutor,
								unsigned char ***OT_Inputs, unsigned char **Xj_checkValues,
								gmp_randstate_t *state, int stat_SecParam, int comp_SecParam)
{
	struct params_CnC_ECC *params_S;

	struct CnC_OT_Mod_CTs **CTs;
	struct CnC_OT_Mod_Check_CT **checkCTs;

	struct tildeCRS *fullTildeCRS;
	struct tildeList *receivedTildeList;
	struct twoDH_Tuples **tuplesList;
	struct jSetCheckTildes *checkTildes;

	unsigned char *commBuffer;
	int i, j, k = 0;
	int commBufferLen, verified = 0, bufferOffset;

	struct timespec int_t_0, int_t_1;
	clock_t int_c_0, int_c_1;


	params_S = setup_CnC_OT_Mod_Full_Sender(writeSocket, readSocket, *state);
	free(commBuffer);


	CTs = (struct CnC_OT_Mod_CTs **) calloc(numInputsExecutor * stat_SecParam, sizeof(struct CnC_OT_Mod_CTs *));


	bufferOffset = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	fullTildeCRS = deserialiseTildeCRS(commBuffer, numInputsExecutor, stat_SecParam, &bufferOffset);
	free(commBuffer);
	
	int_t_0 = timestamp();
	int_c_0 = clock();

	tuplesList = getAllTuplesVerifier(writeSocket, readSocket, params_S, numInputsExecutor, stat_SecParam, fullTildeCRS, state);
	verified = ZKPoK_Verifier_ECC_1Of2_Parallel(writeSocket, readSocket, numInputsExecutor, params_S -> params, tuplesList, state);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Parallel ZKPoK");

	#pragma omp parallel for private(i, j, k) schedule(auto)
	for(i = 0; i < numInputsExecutor; i ++)
	{
		commBufferLen = 0;
		for(j = 0; j < stat_SecParam; j ++)
		{
			k = i * stat_SecParam + j;

			CTs[k] = transfer_CnC_OT_Mod_Enc_i_j(params_S, 16, fullTildeCRS -> lists[i],
												OT_Inputs[0][k], OT_Inputs[1][k],
												*state, j);

		}
	}

	commBuffer = serialise_Mod_CTs(CTs, numInputsExecutor * stat_SecParam, &commBufferLen, 16);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);

	for(j = 0; j < numInputsExecutor * stat_SecParam; j ++)
	{
		clearECC_Point(CTs[j] -> u_0);
		clearECC_Point(CTs[j] -> u_1);
		free(CTs[j] -> w_0);
		free(CTs[j] -> w_1);
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

