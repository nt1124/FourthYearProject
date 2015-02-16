void full_CnC_OT_Sender(int writeSocket, int readSocket, struct Circuit **circuitsArray, gmp_randstate_t *state,
						int stat_SecParam, int comp_SecParam)
{
	struct params_CnC *params_S;
	struct otKeyPair **keyPairs_S;
	struct u_v_Pair **c_i_Array_S;

	struct wire *tempWire;

	unsigned char *commBuffer;
	int i, j, bufferLength = 0, iOffset = 0, numInputsBuilder;
	int totalOTs, u_v_index;

	int k;

	printf("<0><><>\n");
	fflush(stdout);


	commBuffer = receiveBoth(readSocket, bufferLength);
	params_S = setup_CnC_OT_Sender(commBuffer);
	free(commBuffer);

	totalOTs = params_S -> crs -> stat_SecParam * circuitsArray[0] -> numInputsExecutor;
	numInputsBuilder = circuitsArray[0] -> numInputsBuilder;


	printf("<1><><>\n");
	fflush(stdout);

	// When doing this properly the ZKPOK goes here.


	commBuffer = receiveBoth(readSocket, bufferLength);
	keyPairs_S = deserialise_PKs_otKeyPair_Array(commBuffer, totalOTs);
	free(commBuffer);

	printf("<2><><>\n");
	fflush(stdout);


	c_i_Array_S = (struct u_v_Pair **) calloc(2*totalOTs, sizeof(struct u_v_Pair*));
	for(i = numInputsBuilder; i < numInputsBuilder + circuitsArray[0] -> numInputsExecutor; i ++)
	{
		for(j = 0; j < stat_SecParam; j ++)
		{
			tempWire = circuitsArray[j] -> gates[i] -> outputWire;


			CnC_OT_Transfer_One_Sender(tempWire -> outputGarbleKeys -> key0, tempWire -> outputGarbleKeys -> key1, 16,
										params_S, state, keyPairs_S[iOffset + j], c_i_Array_S, u_v_index, j);
			u_v_index += 2;

			printf("<3><%d>\n", i);
			fflush(stdout);
		}
		iOffset += stat_SecParam;
	}

	bufferLength = 0;
	printf("<3><><>\n");
	fflush(stdout);
	commBuffer = serialise_U_V_Pair_Array(c_i_Array_S, totalOTs * 2, &bufferLength);
	printf("<4><><>\n");
	fflush(stdout);
	sendBoth(writeSocket, commBuffer, bufferLength);
	printf("<5><><>\n");
	fflush(stdout);
	free(commBuffer);

	printf("<6><><>\n");
	fflush(stdout);
}