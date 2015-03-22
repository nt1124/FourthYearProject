unsigned char *full_CnC_OT_Receiver_ECC_Alt(int writeSocket, int readSocket, int numInputsExecutor,// struct Circuit **circuitsArray,
											gmp_randstate_t *state, unsigned char *permedInputs,
											unsigned char ***output, int stat_SecParam, int comp_SecParam)
{
	struct params_CnC_ECC *params_R;
	struct otKeyPair_ECC **keyPairs_R;
	struct u_v_Pair_ECC **c_i_Array_R;

	struct wire *tempWire;

	unsigned char *commBuffer, value, *tempChars_0, *tempChars_1;
	int bufferLength = 0, i, j, iOffset = 0;
	int totalOTs = numInputsExecutor * stat_SecParam;
	int u_v_index = 0;
	int k;


	numInputsExecutor = numInputsExecutor;
	*output = (unsigned char **) calloc(2 * totalOTs, sizeof(unsigned char *));

	params_R = setup_CnC_OT_Receiver_ECC(stat_SecParam, comp_SecParam, *state);
	commBuffer = serialiseParams_CnC_ECC(params_R, &bufferLength);
	sendBoth(writeSocket, commBuffer, bufferLength);
	free(commBuffer);

	// Prove that we did indeed select s/2 many to open.
	ZKPoK_Prover_ECC(writeSocket, readSocket, params_R -> params, params_R -> crs -> stat_SecParam,
					params_R -> params -> g, params_R -> crs -> g_1,
					params_R -> crs -> h_0_List, params_R -> crs -> h_1_List,
					params_R -> crs ->  alphas_List, params_R -> crs ->  J_set, state);


	keyPairs_R = (struct otKeyPair_ECC **) calloc(totalOTs, sizeof(struct otKeyPair_ECC*));


	#pragma omp parallel for private(i, j, iOffset, value, tempWire)
	for(i = 0; i < numInputsExecutor; i ++)
	{
		iOffset = stat_SecParam * i;

		value = permedInputs[i];

		for(j = 0; j < stat_SecParam; j ++)
		{
			keyPairs_R[iOffset + j] = CnC_OT_Transfer_One_Receiver_ECC(value, j, params_R, state);
		}
	}

	bufferLength = 0;
	commBuffer = serialise_PKs_otKeyPair_ECC_Array(keyPairs_R, totalOTs, &bufferLength);
	sendBoth(writeSocket, commBuffer, bufferLength);
	free(commBuffer);


	bufferLength = 0;
	commBuffer = receiveBoth(readSocket, bufferLength);
	c_i_Array_R = deserialise_U_V_Pair_Array_ECC(commBuffer, totalOTs * 2);
	free(commBuffer);


	#pragma omp parallel for private(i, j, iOffset, u_v_index, value, tempWire)
	for(i = 0; i < numInputsExecutor; i ++)
	{
		iOffset = stat_SecParam * i;
		u_v_index = 2 * iOffset;

		value = permedInputs[i];

		for(j = 0; j < stat_SecParam; j ++)
		{
			(*output)[u_v_index + 0] = CnC_OT_Output_One_Receiver_0_ECC(c_i_Array_R[u_v_index + 0], value, keyPairs_R[iOffset + j], params_R, j);
			(*output)[u_v_index + 1] = CnC_OT_Output_One_Receiver_1_ECC(c_i_Array_R[u_v_index + 1], value, keyPairs_R[iOffset + j], params_R, j);

			u_v_index += 2;
		}
	}


	for(i = 0; i < totalOTs; i ++)
	{
		freeOT_Pair(keyPairs_R[i]);
		clearECC_Point(c_i_Array_R[i] -> u);
		clearECC_Point(c_i_Array_R[i] -> v);
		free(c_i_Array_R[i]);
	}
	free(keyPairs_R);

	for(; i < 2 * totalOTs; i ++)
	{
		clearECC_Point(c_i_Array_R[i] -> u);
		clearECC_Point(c_i_Array_R[i] -> v);
		free(c_i_Array_R[i]);
	}

	return params_R -> crs -> J_set;
}



void setInputsFromCharArray(struct Circuit **circuitsArray, unsigned char **output, int stat_SecParam)
{
	int i, j, iOffset, u_v_index, numInputsBuilder, numInputsExecutor;
	struct wire *tempWire;
	unsigned char value;


	numInputsBuilder = circuitsArray[0] -> numInputsBuilder;
	numInputsExecutor = circuitsArray[0] -> numInputsExecutor;


	for(i = numInputsBuilder; i < numInputsBuilder + circuitsArray[0] -> numInputsExecutor; i ++)
	{
		iOffset = stat_SecParam * (i - numInputsBuilder);
		u_v_index = 2 * iOffset;

		value = circuitsArray[0] -> gates[i] -> outputWire -> wirePermedValue;
		value = value ^ (circuitsArray[0] -> gates[i] -> outputWire -> wirePerm & 0x01);


		for(j = 0; j < stat_SecParam; j ++)
		{
			tempWire = circuitsArray[j] -> gates[i] -> outputWire;

			tempWire -> outputGarbleKeys -> key0 = output[u_v_index + 0];
			tempWire -> outputGarbleKeys -> key1 = output[u_v_index + 1];

			if(0x00 == value)
			{
				memcpy(tempWire -> wireOutputKey, tempWire -> outputGarbleKeys -> key0, 16);
			}
			else
			{
				memcpy(tempWire -> wireOutputKey, tempWire -> outputGarbleKeys -> key1, 16);
			}

			u_v_index += 2;
		}
	}
}


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
							unsigned char *J_set, int *J_setSize, int stat_SecParam)
{
	struct revealedCheckSecrets *secretsJ_set;
	struct wire *tempWire;
	unsigned char *commBuffer;
	int i, commBufferLen, tempOffset = stat_SecParam * sizeof(unsigned char);
	int count = 0;


	for(i = 0; i < stat_SecParam; i ++)
	{
		count += J_set[i];		
	}
	*J_setSize = count;
	
	commBufferLen = stat_SecParam + (16 * 2) * count;
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
	free(commBuffer);

	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);

	if(1 != commBufferLen)
	{
		secretsJ_set = deserialise_Requested_CircuitSecrets(commBuffer, stat_SecParam, J_set, public_inputs, params);
		free(commBuffer);
		return secretsJ_set;
	}

	return NULL;
}


int secretInputsToCheckCircuits(struct Circuit **circuitsArray, struct RawCircuit *rawInputCircuit,
								struct public_builderPRS_Keys *public_inputs,
								mpz_t *secret_J_set, unsigned int *seedList, struct eccParams *params,
								unsigned char *J_set, int J_setSize, int stat_SecParam)
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
			temp |= compareCircuit(rawInputCircuit, circuitsArray[j], tempGarbleCircuit);

			// printf("Checkpoint Beta 3 => %d\n", j);
			// fflush(stdout);
			freeTempGarbleCircuit(tempGarbleCircuit);
		}
	}

	return temp;
}




void setBuilderInputs(struct eccPoint **builderInputs, unsigned char *J_set, int J_setSize, struct Circuit **circuitsArray,
					struct public_builderPRS_Keys *public_inputs, struct eccParams *params)
{
	unsigned char *rawInput, *hashedInput;
	const int numInputs = public_inputs -> numKeyPairs;
	const int numEvalCircuits = public_inputs -> stat_SecParam;
	int i, j, k = 0;
	int outputLength;



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
										unsigned char *J_set, int J_setSize,
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



	tempU = (struct eccPoint**) calloc(stat_SecParam - J_setSize, sizeof(struct eccPoint*));
	tempV = (struct eccPoint**) calloc(stat_SecParam - J_setSize, sizeof(struct eccPoint*));


	numLambdas = public_inputs -> numKeyPairs * (stat_SecParam - J_setSize);
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
