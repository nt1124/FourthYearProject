void full_CnC_OT_Receiver(int writeSocket, int readSocket, struct Circuit **circuitsArray, gmp_randstate_t *state,
						int stat_SecParam, int comp_SecParam)
{
	struct params_CnC *params_R;
	struct otKeyPair **keyPairs_R;
	struct u_v_Pair **c_i_Array_R;

	struct wire *tempWire;

	unsigned char *commBuffer, value;
	int bufferLength = 0, i, j, iOffset = 0;
	int totalOTs = circuitsArray[0] -> numInputsExecutor * stat_SecParam;

	params_R = setup_CnC_OT_Receiver(stat_SecParam, comp_SecParam, *state);
	commBuffer = serialiseParams_CnC(params_R, &bufferLength);
	sendBoth(writeSocket, commBuffer, bufferLength);
	free(commBuffer);

	// When truly doing it we need to have the ZKPOK here...

	keyPairs_R = (struct otKeyPair **) calloc(totalOTs, sizeof(struct otKeyPair*)
	for(i = 0; i < circuitsArray[0] -> numInputsExecutor; i ++)
	{
		value = circuitsArray[0] -> gates[i] -> outputWire -> wirePermedValue;
		value = value ^ (circuitsArray[0] -> gates[i] -> outputWire -> wirePerm & 0x01);

		for(j = 0; j < stat_SecParam; j ++)
		{
			keyPairs_R[iOffset + j] = CnC_OT_Transfer_One_Receiver(value, j, params_R, state);
		}
		iOffset += circuitsArray[0] -> numInputsExecutor;
	}

	bufferLength = 0;
	commBuffer = serialise_PKs_otKeyPair_Array(keyPairs_R, totalOTs, &bufferLength);
	sendBoth(writeSocket, commBuffer, bufferLength);
	free(commBuffer);


	bufferLength = 0;
	commBuffer = receiveBoth(readSocket, &bufferLength);
	c_i_Array_R = deserialise_U_V_Pair_Array(commBuffer, totalOTs * 2);
	free(commBuffer);

	for(i = 0; i < circuitsArray[0] -> numInputsExecutor; i ++)
	{
		value = circuitsArray[0] -> gates[i] -> outputWire -> wirePermedValue;
		value = value ^ (circuitsArray[0] -> gates[i] -> outputWire -> wirePerm & 0x01);

		for(j = 0; j < stat_SecParam; j ++)
		{
			tempWire = circuitsArray[j] -> gates[i] -> outputWire;
			CnC_OT_Output_One_Receiver(c_i_Array_R[u_v_index], c_i_Array_R[u_v_index + 1], keyPairs_R[i], params_R, value, j, &tempChars_0, &tempChars_1);

			outputBytes[i][0] = tempChars_0;
			outputBytes[i][1] = tempChars_1;
			u_v_index += 2;
		}
	}
}