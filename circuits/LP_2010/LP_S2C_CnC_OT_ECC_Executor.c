struct params_CnC_ECC *OT_CnC_Receiver_Setup_Params(int numInputsExecutor, gmp_randstate_t *state, unsigned char *permedInputs,
													int stat_SecParam, int comp_SecParam)
{
	struct params_CnC_ECC *params_R;
	unsigned char *J_set;


	J_set = generateJ_Set(stat_SecParam);
	params_R = setup_CnC_OT_Receiver_ECC(stat_SecParam, J_set, comp_SecParam, *state);
	
	return params_R;	
}


struct otKeyPair_ECC **OT_CnC_Receiver_Produce_Queries(struct params_CnC_ECC *params_R, int numInputsExecutor, gmp_randstate_t *state,
													unsigned char *permedInputs, int stat_SecParam)
{
	struct otKeyPair_ECC **keyPairs_R;
	unsigned char value;
	int i, j, iOffset;
	int totalOTs = numInputsExecutor * stat_SecParam;


	keyPairs_R = (struct otKeyPair_ECC **) calloc(totalOTs, sizeof(struct otKeyPair_ECC*));

	#pragma omp parallel for private(i, j, iOffset, value)
	for(i = 0; i < numInputsExecutor; i ++)
	{
		iOffset = stat_SecParam * i;

		value = permedInputs[i];

		for(j = 0; j < stat_SecParam; j ++)
		{
			keyPairs_R[iOffset + j] = CnC_OT_Transfer_One_Receiver_ECC(value, j, params_R, state);
		}
	}

	return keyPairs_R;
}

void OT_CnC_Receiver_Send_Queries(int writeSocket, int readSocket,
								struct params_CnC_ECC *params_R, struct otKeyPair_ECC **keyPairs_R, 
								int numInputsExecutor, gmp_randstate_t *state, unsigned char *permedInputs,
								int stat_SecParam)
{
	unsigned char *commBuffer;
	int bufferLength = 0, totalOTs = numInputsExecutor * stat_SecParam;


	commBuffer = serialiseParams_CnC_ECC(params_R, &bufferLength);
	sendBoth(writeSocket, commBuffer, bufferLength);
	free(commBuffer);

	// Prove that we did indeed select s/2 many to open.
	ZKPoK_Prover_ECC(writeSocket, readSocket, params_R -> params, params_R -> crs -> stat_SecParam,
					params_R -> params -> g, params_R -> crs -> g_1,
					params_R -> crs -> h_0_List, params_R -> crs -> h_1_List,
					params_R -> crs ->  alphas_List, params_R -> crs ->  J_set, state);


	bufferLength = 0;
	commBuffer = serialise_PKs_otKeyPair_ECC_Array(keyPairs_R, totalOTs, &bufferLength);
	sendBoth(writeSocket, commBuffer, bufferLength);
	free(commBuffer);
}


unsigned char **OT_CnC_Receiver_Transfer(int writeSocket, int readSocket, struct params_CnC_ECC *params_R,
										struct otKeyPair_ECC **keyPairs_R, int numInputsExecutor, gmp_randstate_t *state,
										unsigned char *permedInputs, int stat_SecParam)
{
	struct u_v_Pair_ECC **c_i_Array_R;
	struct wire *tempWire;

	unsigned char **output, *commBuffer, value;
	int i, j, iOffset, totalOTs = numInputsExecutor * stat_SecParam;
	int bufferLength = 0, u_v_index = 0, k;


	bufferLength = 0;
	commBuffer = receiveBoth(readSocket, bufferLength);
	c_i_Array_R = deserialise_U_V_Pair_Array_ECC(commBuffer, totalOTs * 2);
	free(commBuffer);


	output = (unsigned char **) calloc(2 * totalOTs, sizeof(unsigned char *));

	#pragma omp parallel for private(i, j, iOffset, u_v_index, value, tempWire)
	for(i = 0; i < numInputsExecutor; i ++)
	{
		iOffset = stat_SecParam * i;
		u_v_index = 2 * iOffset;

		value = permedInputs[i];

		for(j = 0; j < stat_SecParam; j ++)
		{
			output[u_v_index + 0] = CnC_OT_Output_One_Receiver_0_ECC(c_i_Array_R[u_v_index + 0], value, keyPairs_R[iOffset + j], params_R, j);
			output[u_v_index + 1] = CnC_OT_Output_One_Receiver_1_ECC(c_i_Array_R[u_v_index + 1], value, keyPairs_R[iOffset + j], params_R, j);

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

	return output;
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
	
	// We want to send the whole J_set and then to send both keys for a wire in each circuit in the J_Set.
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
								mpz_t *secret_J_set, ub4 **circuitSeeds, struct eccParams *params,
								unsigned char *J_set, int J_setSize, int stat_SecParam)
{
	struct Circuit *tempGarbleCircuit;
	struct wire *tempWire;
	struct eccPoint ***consistentInputs;

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

	consistentInputs = getAllConsistentInputsAsPoints(secret_J_set, public_inputs -> public_keyPairs, params, J_set, stat_SecParam, rawInputCircuit -> numInputs_P1);

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

		tempGarbleCircuit = readInCircuit_FromRaw_Seeded_ConsistentInput(tempCTX[j], rawInputCircuit, consistentInputs[k], k, params);

		temp |= compareCircuit(rawInputCircuit, circuitsArray[k], tempGarbleCircuit);

		freeTempGarbleCircuit(tempGarbleCircuit);
	}


	return temp;
}


void setBuilderInputs(struct eccPoint **builderInputs, unsigned char *J_set, int J_setSize, struct Circuit **circuitsArray,
					struct public_builderPRS_Keys *public_inputs, struct eccParams *params)
{
	const int numInputs = public_inputs -> numKeyPairs;
	const int numEvalCircuits = public_inputs -> stat_SecParam;
	int i, j, k = 0;


	for(i = 0; i < numInputs; i ++)
	{
		for(j = 0; j < numEvalCircuits; j ++)
		{
			if(0x00 == J_set[j])
			{
				circuitsArray[j] -> gates[i] -> outputWire -> wireOutputKey = hashECC_Point(builderInputs[k], 16);
				k ++;
			}
		}
	}
}


void setBuilderInputs(struct eccPoint **builderInputs, unsigned char *builderInputBits,
					unsigned char *J_set, int J_setSize, struct Circuit **circuitsArray,
					struct public_builderPRS_Keys *public_inputs, struct eccParams *params)
{
	const int numInputs = public_inputs -> numKeyPairs;
	const int numEvalCircuits = public_inputs -> stat_SecParam;
	int i, j, k = 0;


	for(i = 0; i < numInputs; i ++)
	{
		for(j = 0; j < numEvalCircuits; j ++)
		{
			if(0x00 == J_set[j])
			{
				circuitsArray[j] -> gates[i] -> outputWire -> wireOutputKey = hashECC_Point(builderInputs[k], 16);
				circuitsArray[j] -> gates[i] -> outputWire -> wirePermedValue = builderInputBits[k];
				k ++;
			}
		}
	}
}



int proveConsistencyEvaluationKeys_Exec(int writeSocket, int readSocket,
										unsigned char *J_set, int J_setSize,
										struct eccPoint **builderInputs,
										struct eccPoint ***public_keyPairs, struct eccPoint **public_circuitKeys,
										int numInputKeys, int numCircuitKeys,
										struct eccParams *params, gmp_randstate_t *state)
{
	struct eccPoint **tempU, **tempV;
	unsigned char *commBuffer;
	mpz_t *lambda;

	int i, j, k, l = 0, numLambdas, commBufferLen = 0, lambda_Index = 0;
	int verified = 0, verifiedPrime = 0;


	tempU = (struct eccPoint**) calloc(numCircuitKeys - J_setSize, sizeof(struct eccPoint*));
	tempV = (struct eccPoint**) calloc(numCircuitKeys - J_setSize, sizeof(struct eccPoint*));

	numLambdas = numInputKeys * (numCircuitKeys - J_setSize);
	lambda = (mpz_t *) calloc(numLambdas, sizeof(mpz_t));

	for(i = 0; i < numLambdas; i ++)
	{
		mpz_init(lambda[i]);
		mpz_urandomm(lambda[i], *state, params -> n);
	}
	commBuffer = serialiseMPZ_Array(lambda, numLambdas, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);


	for(i = 0; i < numInputKeys; i ++)
	{
		k = 0;
		for(j = 0; j < numCircuitKeys; j ++)
		{
			if(0x00 == J_set[j])
			{
				tempU[k] = public_circuitKeys[j];
				tempV[k] = builderInputs[l];

				k ++;
				l ++;
			}
		}

		verified |= ZKPoK_Ext_DH_TupleVerifier(writeSocket, readSocket, k,
											params -> g, params -> g,
											public_keyPairs[i][0], public_keyPairs[i][1],
											tempU, tempV, params, state,
											lambda, lambda_Index);

		lambda_Index += k;
	}

	return verified;
}


