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


	params_R = setup_CnC_OT_Receiver(stat_SecParam, comp_SecParam, *state);
	commBuffer = serialiseParams_CnC(params_R, &bufferLength);
	sendBoth(writeSocket, commBuffer, bufferLength);
	free(commBuffer);


	// When truly doing it we need to have the ZKPOK here...


	keyPairs_R = (struct otKeyPair **) calloc(totalOTs, sizeof(struct otKeyPair*));
	numInputsBuilder = circuitsArray[0] -> numInputsBuilder;

	#pragma omp parallel for private(i, j, iOffset, value)
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

	iOffset = 0;


	#pragma omp parallel for private(i, j, iOffset, u_v_index, tempWire, value, tempChars_0, tempChars_1)
	for(i = numInputsBuilder; i < numInputsBuilder + circuitsArray[0] -> numInputsExecutor; i ++)
	{
		iOffset = stat_SecParam * (i - numInputsBuilder);
		u_v_index = iOffset * 2;

		value = circuitsArray[0] -> gates[i] -> outputWire -> wirePermedValue;
		value = value ^ (circuitsArray[0] -> gates[i] -> outputWire -> wirePerm & 0x01);

		for(j = 0; j < stat_SecParam; j ++)
		{
			tempWire = circuitsArray[j] -> gates[i] -> outputWire;

			tempChars_0 = CnC_OT_Output_One_Receiver_0(c_i_Array_R[u_v_index + 0], value, keyPairs_R[iOffset + j], params_R, j);
			tempChars_1 = CnC_OT_Output_One_Receiver_1(c_i_Array_R[u_v_index + 1], value, keyPairs_R[iOffset + j], params_R, j);

			tempWire -> outputGarbleKeys -> key0 = tempChars_0;
			tempWire -> outputGarbleKeys -> key1 = tempChars_1;

			if(0x00 == value)
			{
				memcpy(tempWire -> wireOutputKey, tempChars_0, 16);
			}
			else
			{
				memcpy(tempWire -> wireOutputKey, tempChars_1, 16);
			}

			u_v_index += 2;
		}
	}
}