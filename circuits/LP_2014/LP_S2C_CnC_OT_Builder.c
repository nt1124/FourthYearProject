
void full_CnC_OT_Sender(int writeSocket, int readSocket, struct Circuit **circuitsArray, gmp_randstate_t *state,
						int stat_SecParam, int comp_SecParam)
{
	struct params_CnC *params_S;
	struct u_v_Pair **c_i_Array_S;

	struct wire *tempWire;

	unsigned char *commBuffer;
	int i, j, bufferLength = 0, iOffset = 0;
	int totalOTs, u_v_index;


	commBuffer = receiveBoth(readSocket, &bufferLength);

	params_S = setup_CnC_OT_Sender(commBuffer);
	free(commBuffer);

	totalOTs = params_S -> crs -> stat_SecParam * circuitsArray[0] -> numInputsExecutor;

	// When doing this properly the ZKPOK goes here.

	commBuffer = receiveBoth(readSocket, &bufferLength);
	keyPairs_S = deserialise_PKs_otKeyPair_Array(commBuffer, totalOTs);
	free(commBuffer);
			

	c_i_Array_S = (struct u_v_Pair **) calloc(2*totalOTs, sizeof(struct u_v_Pair*));
	for(i = 0; i < circuitsArray[0] -> numInputsExecutor; i ++)
	{
		for(j = 0; j < stat_SecParam; j ++)
		{
			tempWire = circuitsArray[j] -> gates[i] -> outputWire;
			CnC_OT_Transfer_One_Sender(tempWire -> outputGarbleKeys -> key0, tempWire -> outputGarbleKeys -> key1, 16,
										params_S, state, keyPairs_S[iOffset + j], c_i_Array_S, u_v_index, j);
			u_v_index += 2;
			iOffset += circuitsArray[0] -> numInputsExecutor;
		}
	}

	bufferLength = 0;
	commBuffer = serialise_U_V_Pair_Array(c_i_Array_S, totalOTs * 2, &bufferLength);
	sendBoth(writeSocket, commBuffer, bufferLength);
	free(commBuffer);
}