unsigned char *full_CnC_OT_Receiver_ECC(int writeSocket, int readSocket, struct Circuit **circuitsArray, gmp_randstate_t *state,
						int stat_SecParam, int comp_SecParam)
{
	struct params_CnC_ECC *params_R;
	struct otKeyPair_ECC **keyPairs_R;
	struct u_v_Pair_ECC **c_i_Array_R;

	struct wire *tempWire;

	unsigned char *commBuffer, value, *tempChars_0, *tempChars_1;
	int bufferLength = 0, i, j, iOffset = 0, numInputsBuilder;
	int totalOTs = circuitsArray[0] -> numInputsExecutor * stat_SecParam;
	int u_v_index = 0;
	int k;


	params_R = setup_CnC_OT_Receiver_ECC(stat_SecParam, comp_SecParam, *state);
	commBuffer = serialiseParams_CnC_ECC(params_R, &bufferLength);
	sendBoth(writeSocket, commBuffer, bufferLength);
	free(commBuffer);

	// Prove that we did indeed select s/2 many to open.
	ZKPoK_Prover_ECC(writeSocket, readSocket, params_R -> params, params_R -> crs -> stat_SecParam,
					params_R -> crs -> g_1, params_R -> crs -> h_0_List, params_R -> crs -> h_1_List,
					params_R -> crs ->  alphas_List, params_R -> crs ->  J_set, state);


	keyPairs_R = (struct otKeyPair_ECC **) calloc(totalOTs, sizeof(struct otKeyPair_ECC*));
	numInputsBuilder = circuitsArray[0] -> numInputsBuilder;


	#pragma omp parallel for private(i, j, iOffset, value, tempWire)
	for(i = numInputsBuilder; i < numInputsBuilder + circuitsArray[0] -> numInputsExecutor; i ++)
	{
		iOffset = stat_SecParam * (i - numInputsBuilder);

		value = circuitsArray[0] -> gates[i] -> outputWire -> wirePermedValue;
		value = value ^ (circuitsArray[0] -> gates[i] -> outputWire -> wirePerm & 0x01);


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
	for(i = numInputsBuilder; i < numInputsBuilder + circuitsArray[0] -> numInputsExecutor; i ++)
	{
		iOffset = stat_SecParam * (i - numInputsBuilder);
		u_v_index = 2 * iOffset;

		value = circuitsArray[0] -> gates[i] -> outputWire -> wirePermedValue;
		value = value ^ (circuitsArray[0] -> gates[i] -> outputWire -> wirePerm & 0x01);


		for(j = 0; j < stat_SecParam; j ++)
		{
			tempWire = circuitsArray[j] -> gates[i] -> outputWire;

			tempWire -> outputGarbleKeys -> key0 = CnC_OT_Output_One_Receiver_0_ECC(c_i_Array_R[u_v_index + 0], value, keyPairs_R[iOffset + j], params_R, j);
			tempWire -> outputGarbleKeys -> key1 = CnC_OT_Output_One_Receiver_1_ECC(c_i_Array_R[u_v_index + 1], value, keyPairs_R[iOffset + j], params_R, j);

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

	return params_R -> crs -> J_set;
}


struct publicInputsWithGroup *receivePublicCommitments_ECC(int writeSocket, int readSocket)
{
	struct publicInputsWithGroup *toReturn = (struct publicInputsWithGroup *) calloc(1, sizeof(struct publicInputsWithGroup));
	unsigned char *commBuffer;
	int commBufferLen = 0, bufferOffset = 0;

	commBuffer = receiveBoth(readSocket, commBufferLen);

	toReturn -> public_inputs = deserialisePublicInputs(commBuffer, &bufferOffset);
	toReturn -> group = deserialise_DDH_Group(commBuffer, &bufferOffset);


	free(commBuffer);

	return toReturn;
}


struct revealedCheckSecrets *executor_decommitToJ_Set_ECC(int writeSocket, int readSocket, struct Circuit **circuitsArray,
							struct public_builderPRS_Keys *public_inputs, struct DDH_Group *group,
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

	// printf("Checkpoint 3\n");
	// fflush(stdout);

	sendBoth(writeSocket, commBuffer, commBufferLen);
	// free(commBuffer);

	// printf("Checkpoint 4  -  %d\n", commBufferLen);
	// fflush(stdout);

	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);

	// printf("Checkpoint 5  -  %d\n", commBufferLen);
	// fflush(stdout);


	if(1 != commBufferLen)
	{
		secretsJ_set = deserialise_Requested_CircuitSecrets(commBuffer, stat_SecParam, J_set, public_inputs, group);
		return secretsJ_set;
	}

	return NULL;
}


int secretInputsToCheckCircuits_ECC(struct Circuit **circuitsArray, struct RawCircuit *rawInputCircuit,
								struct public_builderPRS_Keys *public_inputs,
								mpz_t *secret_J_set, unsigned int *seedList, struct DDH_Group *group,
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

				tempWire -> outputGarbleKeys -> key0 = compute_Key_b_Input_i_Circuit_j(secret_J_set[j], public_inputs, group, i, 0x00);
				tempWire -> outputGarbleKeys -> key1 = compute_Key_b_Input_i_Circuit_j(secret_J_set[j], public_inputs, group, i, 0x01);
			}

			tempGarbleCircuit = readInCircuit_FromRaw_Seeded_ConsistentInput(rawInputCircuit, seedList[j], secret_J_set[j], public_inputs, j, group);
			temp = compareCircuit(rawInputCircuit, circuitsArray[j], tempGarbleCircuit);
		}
	}

	return 1;
}