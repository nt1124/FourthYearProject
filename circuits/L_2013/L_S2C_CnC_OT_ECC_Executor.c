unsigned char *full_CnC_OT_Mod_Receiver_ECC(int writeSocket, int readSocket, struct Circuit **circuitsArray, gmp_randstate_t *state,
						int stat_SecParam, int comp_SecParam)
{
	struct params_CnC_ECC *params_R;
	struct tildeList *testTildeList;
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


	for(i = numInputsBuilder; i < numInputsBuilder + circuitsArray[0] -> numInputsExecutor; i ++)
	{
		iOffset = stat_SecParam * (i - numInputsBuilder);

		value = circuitsArray[0] -> gates[i] -> outputWire -> wirePermedValue;
		value = value ^ (circuitsArray[0] -> gates[i] -> outputWire -> wirePerm & 0x01);

		mpz_urandomm(r_i, *state, params_R -> params -> n);
		testTildeList = initTildeList(stat_SecParam, r_i, params_R -> crs, params_R -> params, value);

		commBufferLen = 0;
		commBuffer = serialiseTildeList(testTildeList, stat_SecParam, &commBufferLen);
		sendBoth(writeSocket, commBuffer, commBufferLen);
		free(commBuffer);

		// ZKPoK Here

		ZKPoK_Ext_DH_TupleProver_2U(writeSocket, readSocket, stat_SecParam, &r_i, value,
									params_R -> params -> g, params_R -> crs -> g_1,
									testTildeList -> g_tilde, testTildeList -> g_tilde,
									params_R -> crs -> h_0_List, params_R -> crs -> h_1_List,
									testTildeList -> h_tildeList, params_R -> params, state);

		bufferOffset = 0;
		commBuffer = receiveBoth(readSocket, commBufferLen);
		CTs = deserialise_Mod_CTs(commBuffer, &bufferOffset, 16);
		free(commBuffer);


		// #pragma omp parallel for private(j, tempWire) schedule(auto)
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

		/*
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
		*/
	}


	return params_R -> crs -> J_set;
}



/*
struct publicInputsWithGroup *receivePublicCommitments(int writeSocket, int readSocket)
{
	struct publicInputsWithGroup *toReturn = (struct publicInputsWithGroup *) calloc(1, sizeof(struct publicInputsWithGroup));
	unsigned char *commBuffer;
	int commBufferLen = 0, bufferOffset = 0;

	commBuffer = receiveBoth(readSocket, commBufferLen);

	toReturn -> public_inputs = deserialisePublicInputs(commBuffer, &bufferOffset);
	toReturn -> params = deserialiseECC_Params(commBuffer, &bufferOffset);


	free(commBuffer);

	return toReturn;
}


struct revealedCheckSecrets *executor_decommitToJ_Set(int writeSocket, int readSocket, struct Circuit **circuitsArray,
							struct public_builderPRS_Keys *public_inputs, struct eccParams *params,
							unsigned char *J_set, int stat_SecParam)
{
	struct revealedCheckSecrets *secretsJ_set;
	struct wire *tempWire;
	unsigned char *commBuffer;
	int i, commBufferLen, tempOffset = stat_SecParam * sizeof(unsigned char);

	commBufferLen = stat_SecParam + (16 * 2) * (stat_SecParam / 2);
	commBuffer = (unsigned char*) calloc(commBufferLen, sizeof(unsigned char));

	memcpy(commBuffer, J_set, stat_SecParam * sizeof(unsigned char));


	// For each circuit we send both value on our first input wire wire, thus proving that
	// we indeed opened that circuit for checking (i.e. It's J_set value was 1).
	for(i = 0; i < stat_SecParam; i ++)
	{
		if(0x01 == J_set[i])
		{
			tempWire = circuitsArray[i] -> gates[circuitsArray[i] -> numInputsBuilder] -> outputWire;

			memcpy(commBuffer + tempOffset, tempWire -> outputGarbleKeys -> key0, 16);
			tempOffset += 16;

			memcpy(commBuffer + tempOffset, tempWire -> outputGarbleKeys -> key1, 16);
			tempOffset += 16;
		}
	}


	sendBoth(writeSocket, commBuffer, commBufferLen);
	// free(commBuffer);

	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);

	if(1 != commBufferLen)
	{
		secretsJ_set = deserialise_Requested_CircuitSecrets(commBuffer, stat_SecParam, J_set, public_inputs, params);
		return secretsJ_set;
	}

	return NULL;
}


int secretInputsToCheckCircuits(struct Circuit **circuitsArray, struct RawCircuit *rawInputCircuit,
								struct public_builderPRS_Keys *public_inputs,
								mpz_t *secret_J_set, unsigned int *seedList, struct eccParams *params,
								unsigned char *J_set, int stat_SecParam)
{
	struct Circuit *tempGarbleCircuit;
	struct wire *tempWire;
	int i, j, temp = 0;


	// #pragma omp parallel for default(shared) private(i, j, tempWire, tempGarbleCircuit) reduction(+:temp) 
	for(j = 0; j < stat_SecParam; j ++)
	{
		if(0x01 == J_set[j])
		{
			for(i = 0; i < rawInputCircuit -> numInputsBuilder; i ++)
			{
				tempWire = circuitsArray[j] -> gates[i] -> outputWire;
				tempWire -> outputGarbleKeys = (struct bitsGarbleKeys*) calloc(1, sizeof(struct bitsGarbleKeys));

				tempWire -> outputGarbleKeys -> key0 = compute_Key_b_Input_i_Circuit_j(secret_J_set[j], public_inputs, params, i, 0x00);
				tempWire -> outputGarbleKeys -> key1 = compute_Key_b_Input_i_Circuit_j(secret_J_set[j], public_inputs, params, i, 0x01);
			}

			tempGarbleCircuit = readInCircuit_FromRaw_Seeded_ConsistentInput(rawInputCircuit, seedList[j], secret_J_set[j], public_inputs, j, params);
			temp = compareCircuit(rawInputCircuit, circuitsArray[j], tempGarbleCircuit);
		
			//freeCircuitStruct(tempGarbleCircuit, 0);
			freeTempGarbleCircuit(tempGarbleCircuit);
		}
	}

	return 1;
}




void setBuilderInputs(struct eccPoint **builderInputs, unsigned char *J_set, struct Circuit **circuitsArray,
					struct public_builderPRS_Keys *public_inputs, struct eccParams *params)
{
	unsigned char *rawInput, *hashedInput;
	const int numInputs = public_inputs -> numKeyPairs;
	const int numEvalCircuits = public_inputs -> stat_SecParam;
	int i, j, k = 0;
	int outputLength;

	int h;


	for(i = 0; i < numInputs; i ++)
	{
		for(j = 0; j < numEvalCircuits; j ++)
		{
			if(0x00 == J_set[j])
			{
				outputLength = 0;
				rawInput = convertMPZToBytes(builderInputs[k] -> x, &outputLength);
				hashedInput = sha256_full(rawInput, outputLength);
				//circuitsArray[j] -> gates[i] -> outputWire -> wireOutputKey = sha256_full(rawInput, outputLength);

				memcpy(circuitsArray[j] -> gates[i] -> outputWire -> wireOutputKey, hashedInput, 16);
				k ++;

				free(hashedInput);
				free(rawInput);
			}
		}
	}
}





int proveConsistencyEvaluationKeys_Exec(int writeSocket, int readSocket,
										unsigned char *J_set,
										struct eccPoint **builderInputs,
										struct public_builderPRS_Keys *public_inputs,
										struct eccParams *params, gmp_randstate_t *state)
{
	struct eccPoint **tempU, **tempV;
	unsigned char *commBuffer;
	mpz_t *lambda;

	int i, j, k, l = 0, numLambdas, commBufferLen = 0, lambda_Index = 0;
	int verified = 0;
	const int stat_SecParam = public_inputs -> stat_SecParam;



	tempU = (struct eccPoint**) calloc(stat_SecParam / 2, sizeof(struct eccPoint*));
	tempV = (struct eccPoint**) calloc(stat_SecParam / 2, sizeof(struct eccPoint*));


	numLambdas = public_inputs -> numKeyPairs * stat_SecParam / 2;
	lambda = (mpz_t *) calloc(numLambdas, sizeof(mpz_t));

	for(i = 0; i < numLambdas; i ++)
	{
		mpz_init(lambda[i]);
		mpz_urandomm(lambda[i], *state, params -> n);
	}
	commBuffer = serialiseMPZ_Array(lambda, numLambdas, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);


	for(i = 0; i < public_inputs -> numKeyPairs; i ++)
	{
		k = 0;
		for(j = 0; j < stat_SecParam; j ++)
		{
			if(0x00 == J_set[j])
			{
				tempU[k] = public_inputs -> public_circuitKeys[j];
				tempV[k] = builderInputs[l];

				k ++;
				l ++;
			}
		}


		verified |= ZKPoK_Ext_DH_TupleVerifier(writeSocket, readSocket, k,
											params -> g, params -> g,
											public_inputs -> public_keyPairs[i][0],
											public_inputs -> public_keyPairs[i][1],
											tempU, tempV, params, state,
											lambda, lambda_Index);

		lambda_Index += k;
	}

	return verified;
}
*/
