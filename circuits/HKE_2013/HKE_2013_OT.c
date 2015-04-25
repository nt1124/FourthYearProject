struct eccPoint *exchangeC_ForNaorPinkas(int writeSocket, int readSocket, struct eccPoint *C)
{
	struct eccPoint *cTilde;
	unsigned char *commBuffer;
	int commBufferLen, bufferOffset;



	bufferOffset = 0;
	commBufferLen = sizeOfSerial_ECCPoint(C);

	commBuffer = (unsigned char *) calloc(commBufferLen, sizeof(unsigned char));
	serialise_ECC_Point(C, commBuffer, &bufferOffset);
	sendBoth(writeSocket, commBuffer, bufferOffset);
	free(commBuffer);


	commBufferLen = 0;
	bufferOffset = 0;

	commBuffer = receiveBoth(readSocket, commBufferLen);
	cTilde = deserialise_ECC_Point(commBuffer, &bufferOffset);
	free(commBuffer);


	return cTilde;
}


struct OT_NP_Receiver_Query **NaorPinkas_OT_Produce_Queries(int numInputsExecutor, unsigned char *permedInputs,
															gmp_randstate_t *state, struct eccParams *params, struct eccPoint *cTilde)
{
	struct OT_NP_Receiver_Query **queries_R;
	unsigned char sigmaBit;
	int i;


	queries_R = (struct OT_NP_Receiver_Query **) calloc(numInputsExecutor, sizeof(struct OT_NP_Receiver_Query *));
	
	for(i = 0; i < numInputsExecutor; i ++)
	{
		sigmaBit = permedInputs[i];
		queries_R[i] = OT_NP_query(cTilde, sigmaBit, params, *state);
	}

	return queries_R;
}


struct eccPoint **NaorPinkas_OT_Exchange_Queries(int writeSocket, int readSocket, int numOwnQueries, int numPartnerQueries,
										struct OT_NP_Receiver_Query **queries_Own)
{
	struct eccPoint **queries_Partner;
	unsigned char *commBuffer;
	int commBufferLen;


	commBufferLen = 0;
	commBuffer = serialiseQueries(queries_Own, numOwnQueries, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);

	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	queries_Partner = deserialiseQueries(commBuffer, numPartnerQueries);
	free(commBuffer);


	return queries_Partner;
}


void NaorPinkas_OT_Sender_Transfer(int writeSocket, int readSocket, int numInputsExecutor,
							unsigned char ***OT_Inputs, gmp_randstate_t *state, int numCircuits,
							struct eccPoint **queries_S, struct eccParams *params, struct eccPoint *C)
{
	struct OT_NP_Sender_Transfer **transfers_S;

	unsigned char *commBuffer;
	int i, j, k = 0, numOTs;
	int commBufferLen, verified = 0, bufferOffset;

	struct timespec int_t_0, int_t_1;
	clock_t int_c_0, int_c_1;
	int h;


	numOTs = numInputsExecutor * numCircuits;

	transfers_S = (struct OT_NP_Sender_Transfer **) calloc(numOTs, sizeof(struct OT_NP_Sender_Transfer *));

	#pragma omp parallel for private(i, j, k) schedule(auto)
	for(i = 0; i < numInputsExecutor; i ++)
	{
		for(j = 0; j < numCircuits; j ++)
		{
			k = i * numCircuits + j;

			transfers_S[k] = OT_NP_Transfer(C, queries_S[i], OT_Inputs[0][k], OT_Inputs[1][k], 16, *state, params);
		}
	}

	commBufferLen = 0;
	commBuffer = serialiseTransferStructs(transfers_S, numOTs, 16, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);

	for(i = 0; i < numOTs; i ++)
	{
		clearECC_Point(transfers_S[i] -> a);
		free(transfers_S[i] -> c_0);
		free(transfers_S[i] -> c_1);
		free(transfers_S[i]);
	}
}


unsigned char **NaorPinkas_OT_Receiver_Transfer(int writeSocket, int readSocket, int numInputsExecutor,
							unsigned char *permedInputs, gmp_randstate_t *state, int numCircuits,
							struct OT_NP_Receiver_Query **queries_R, struct eccParams *params, struct eccPoint *C)
{
	struct OT_NP_Sender_Transfer **transfers_R;

	unsigned char *commBuffer, sigmaBit, **outputBytes;
	int i, j, k = 0, numOTs;
	int commBufferLen, verified = 0, bufferOffset;

	struct timespec int_t_0, int_t_1;
	clock_t int_c_0, int_c_1;
	int h;


	numOTs = numInputsExecutor * numCircuits;


	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	transfers_R = deserialiseTransferStructs(commBuffer, numOTs, 16);
	free(commBuffer);

	outputBytes = (unsigned char **) calloc(numOTs, sizeof(unsigned char *));
	#pragma omp parallel for private(i, j, k, sigmaBit) schedule(auto)
	for(i = 0; i < numInputsExecutor; i ++)
	{
		sigmaBit = permedInputs[i];

		for(j = 0; j < numCircuits; j ++)
		{
			k = i * numCircuits + j;

			outputBytes[k] = OT_NP_Output_Xb(C, transfers_R[k] -> a, queries_R[i] -> k, transfers_R[k] -> c_0, transfers_R[k] -> c_1, sigmaBit, 16, params);
		}
	}

	for(i = 0; i < numOTs; i ++)
	{
		clearECC_Point(transfers_R[i] -> a);
		free(transfers_R[i] -> c_0);
		free(transfers_R[i] -> c_1);
		free(transfers_R[i]);
	}



	return outputBytes;
}