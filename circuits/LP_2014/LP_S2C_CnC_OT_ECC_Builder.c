void full_CnC_OT_Sender_ECC(int writeSocket, int readSocket, struct Circuit **circuitsArray, gmp_randstate_t *state,
						int stat_SecParam, int comp_SecParam)
{
	struct params_CnC_ECC *params_S;
	struct otKeyPair_ECC **keyPairs_S;
	struct u_v_Pair_ECC **c_i_Array_S;

	struct wire *tempWire;

	unsigned char *commBuffer;
	int i, j, bufferLength = 0, iOffset = 0, numInputsBuilder;
	int totalOTs, u_v_index;

	int k, tupleVerified = 0;


	commBuffer = receiveBoth(readSocket, bufferLength);
	params_S = setup_CnC_OT_Sender_ECC(commBuffer);
	free(commBuffer);

	totalOTs = params_S -> crs -> stat_SecParam * circuitsArray[0] -> numInputsExecutor;
	numInputsBuilder = circuitsArray[0] -> numInputsBuilder;


	// When doing this properly the ZKPOK goes here.
	tupleVerified = ZKPoK_Verifier_ECC(writeSocket, readSocket, params_S -> params, params_S -> crs -> stat_SecParam,
									params_S -> params -> g, params_S -> crs -> g_1,
									params_S -> crs -> h_0_List, params_S -> crs -> h_1_List,
									state);
	// int stat_SecParam, struct eccParams *params, struct eccPoint *g_1, struct eccPoint **h_0_List, struct eccPoint **h_1_List

	commBuffer = receiveBoth(readSocket, bufferLength);
	keyPairs_S = deserialise_PKs_otKeyPair_ECC_Array(commBuffer, totalOTs);
	free(commBuffer);


	c_i_Array_S = (struct u_v_Pair_ECC **) calloc(2*totalOTs, sizeof(struct u_v_Pair_ECC*));

	#pragma omp parallel for ordered private(i, j, iOffset, u_v_index, tempWire) schedule(auto)
	for(i = numInputsBuilder; i < numInputsBuilder + circuitsArray[0] -> numInputsExecutor; i ++)
	{
		// #pragma omp ordered
		{
			iOffset = stat_SecParam * (i - numInputsBuilder);
			u_v_index = 2 * iOffset;

			for(j = 0; j < stat_SecParam; j ++)
			{
				tempWire = circuitsArray[j] -> gates[i] -> outputWire;

				CnC_OT_Transfer_One_Sender_ECC(tempWire -> outputGarbleKeys -> key0, tempWire -> outputGarbleKeys -> key1, 16,
											params_S, state, keyPairs_S[iOffset + j], c_i_Array_S, u_v_index, j);
				u_v_index += 2;
			}
		}
	}

	bufferLength = 0;
	commBuffer = serialise_U_V_Pair_Array_ECC(c_i_Array_S, totalOTs * 2, &bufferLength);
	sendBoth(writeSocket, commBuffer, bufferLength);

	free(commBuffer);
}



void sendPublicCommitments_ECC(int writeSocket, int readSocket, struct public_builderPRS_Keys *public_inputs, struct DDH_Group *group)
{
	unsigned char *publicInputsBytes, *groupBytes, *finalBuffer;
	int publicInputsLen = 0, groupLen = 0, finalLength;


	publicInputsBytes = serialisePublicInputs(public_inputs, &publicInputsLen);
	groupBytes = serialiseDDH_Group(group, &groupLen);


	finalLength = publicInputsLen + groupLen;
	finalBuffer = (unsigned char *) calloc(finalLength, sizeof(unsigned char));


	memcpy(finalBuffer, publicInputsBytes, publicInputsLen);
	memcpy(finalBuffer + publicInputsLen, groupBytes, groupLen);

	sendBoth(writeSocket, finalBuffer, finalLength);

	free(finalBuffer);
}


void builder_decommitToJ_Set_ECC(int writeSocket, int readSocket, struct Circuit **circuitsArray,
							struct secret_builderPRS_Keys *secret_Inputs, int stat_SecParam,
							unsigned int *seedList)
{
	struct wire *tempWire;
	unsigned char *commBuffer, *J_Set;
	int tempOffset = circuitsArray[0] -> numInputs, commBufferLen = 0;
	unsigned char key0_Correct, key1_Correct, finalOutput = 0x00;
	int i;


	J_Set = (unsigned char *) calloc(stat_SecParam, sizeof(unsigned char));

	commBuffer = receiveBoth(readSocket, commBufferLen);
	memcpy(J_Set, commBuffer, stat_SecParam);


	for(i = 0; i < stat_SecParam; i ++)
	{
		if(0x01 == J_Set[i])
		{
			tempWire = circuitsArray[i] -> gates[circuitsArray[i] -> numInputsBuilder] -> outputWire;

			key0_Correct = memcmp(commBuffer + tempOffset, tempWire -> outputGarbleKeys -> key0, 16);
			tempOffset += 16;

			key1_Correct = memcmp(commBuffer + tempOffset, tempWire -> outputGarbleKeys -> key1, 16);
			tempOffset += 16;

			finalOutput = finalOutput | key0_Correct | key1_Correct;
		}
	}
	free(commBuffer);

	if(0x00 != finalOutput)
	{
		commBufferLen = 0;
		commBuffer = serialise_Requested_CircuitSecrets(secret_Inputs, seedList, J_Set, &commBufferLen);

		sendBoth(writeSocket, commBuffer, commBufferLen);
	}
	else
	{
		commBuffer = (unsigned char *) calloc(1, sizeof(unsigned char));
		sendBoth(writeSocket, commBuffer, 1);
	}
}