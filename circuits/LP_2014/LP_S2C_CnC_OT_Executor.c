void full_CnC_OT_Receiver(int writeSocket, int readSocket, struct Circuit **circuitsArray, gmp_randstate_t *state,
						int stat_SecParam, int comp_SecParam)
{
	struct params_CnC *params_R;
	struct otKeyPair **keyPairs_R;
	struct u_v_Pair **c_i_Array_R;

	struct wire *tempWire;

	unsigned char *commBuffer, value, *tempChars_0, *tempChars_1;
	int bufferLength = 0, i, j, iOffset = 0, numInputsBuilder;
	int totalOTs = circuitsArray[0] -> numInputsExecutor * stat_SecParam;
	int u_v_index = 0;
	int k;


	params_R = setup_CnC_OT_Receiver(stat_SecParam, comp_SecParam, *state);
	commBuffer = serialiseParams_CnC(params_R, &bufferLength);
	sendBoth(writeSocket, commBuffer, bufferLength);
	free(commBuffer);


	// When truly doing it we need to have the ZKPOK here...


	keyPairs_R = (struct otKeyPair **) calloc(totalOTs, sizeof(struct otKeyPair*));
	numInputsBuilder = circuitsArray[0] -> numInputsBuilder;

	#pragma omp parallel for private(i, j, iOffset, value, tempWire)
	for(i = numInputsBuilder; i < numInputsBuilder + circuitsArray[0] -> numInputsExecutor; i ++)
	{
		iOffset = stat_SecParam * (i - numInputsBuilder);

		value = circuitsArray[0] -> gates[i] -> outputWire -> wirePermedValue;
		value = value ^ (circuitsArray[0] -> gates[i] -> outputWire -> wirePerm & 0x01);


		for(j = 0; j < stat_SecParam; j ++)
		{
			keyPairs_R[iOffset + j] = CnC_OT_Transfer_One_Receiver(value, j, params_R, state);
		}
	}

	bufferLength = 0;
	commBuffer = serialise_PKs_otKeyPair_Array(keyPairs_R, totalOTs, &bufferLength);
	sendBoth(writeSocket, commBuffer, bufferLength);
	free(commBuffer);


	bufferLength = 0;
	commBuffer = receiveBoth(readSocket, bufferLength);
	c_i_Array_R = deserialise_U_V_Pair_Array(commBuffer, totalOTs * 2);
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

			tempWire -> outputGarbleKeys -> key0 = CnC_OT_Output_One_Receiver_0(c_i_Array_R[u_v_index + 0], value, keyPairs_R[iOffset + j], params_R, j);
			tempWire -> outputGarbleKeys -> key1 = CnC_OT_Output_One_Receiver_1(c_i_Array_R[u_v_index + 1], value, keyPairs_R[iOffset + j], params_R, j);

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


void receivePublicCommitments(int writeSocket, int readSocket, struct public_builderPRS_Keys *public_inputs, struct DDH_Group *group)
{
	unsigned char *commBuffer;
	int commBufferLen = 0, bufferOffset = 0;

	commBuffer = receiveBoth(readSocket, commBufferLen);

	public_inputs = deserialisePublicInputs(commBuffer, &bufferOffset);
	group = deserialise_DDH_Group(commBuffer, &bufferOffset);


	free(commBuffer);
}


mpz_t **executor_decommitToJ_Set(int writeSocket, int readSocket, struct Circuit **circuitsArray,
							struct public_builderPRS_Keys *public_inputs, struct DDH_Group *group,
							unsigned char *J_Set, int stat_SecParam)
{
	mpz_t **secret_J_Set;
	struct wire *tempWire;
	unsigned char *commBuffer;
	int i, commBufferLen, tempOffset = stat_SecParam;

	commBufferLen = stat_SecParam + 16 * stat_SecParam;
	commBuffer = (unsigned char*) calloc(commBufferLen, sizeof(unsigned char));

	memcpy(commBuffer, J_Set, stat_SecParam);

	for(i = 0; i < stat_SecParam; i ++)
	{
		if(0x01 == J_Set[i])
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

	commBuffer = receiveBoth(readSocket, commBufferLen);

	if(1 != commBufferLen)
	{
		secret_J_Set = deserialise_Requested_CircuitSecrets(commBuffer, stat_SecParam, J_Set, public_inputs, group);
		return secret_J_Set;
	}

	return NULL;
}


void secretInputsToCheckCircuits(struct Circuit **circuitsArray, struct public_builderPRS_Keys *public_inputs,
								mpz_t **secret_J_Set, struct DDH_Group *group,
								unsigned char *J_Set, int stat_SecParam)
{
	struct wire *tempWire;
	int i, j;

	for(j = 0; j < stat_SecParam; j ++)
	{
		if(0x01 == J_Set[i])
		{
			for(i = 0; i < circuitsArray[j] -> numInputsBuilder; i ++)
			{
				tempWire = circuitsArray[j] -> gates[i] -> outputWire;
				tempWire -> outputGarbleKeys = (struct bitsGarbleKeys*) calloc(1, sizeof(struct bitsGarbleKeys));

				tempWire -> outputGarbleKeys -> key0 = compute_Key_b_Input_i_Circuit_j(*(secret_J_Set[j]), public_inputs, group, i, 0x00);
				tempWire -> outputGarbleKeys -> key1 = compute_Key_b_Input_i_Circuit_j(*(secret_J_Set[j]), public_inputs, group, i, 0x01);
			}
		}
	}
}