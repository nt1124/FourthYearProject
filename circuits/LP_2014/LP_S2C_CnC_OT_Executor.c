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