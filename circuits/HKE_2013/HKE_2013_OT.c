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





struct OT_NP_Receiver_Query **NaorPinkas_OT_Receiver_Queries(int writeSocket, int readSocket, int numInputsExecutor,
							unsigned char *permedInputs, gmp_randstate_t *state, struct eccParams *params, struct eccPoint *C)
{
	struct OT_NP_Receiver_Query **queries_R;

	unsigned char *commBuffer, sigmaBit, **outputBytes;
	int i, commBufferLen;


	queries_R = (struct OT_NP_Receiver_Query **) calloc(numInputsExecutor, sizeof(struct OT_NP_Receiver_Query *));
	
	for(i = 0; i < numInputsExecutor; i ++)
	{
		sigmaBit = permedInputs[i];
		queries_R[i] = OT_NP_query(C, sigmaBit, params, *state);
	}

	commBufferLen = 0;
	commBuffer = serialiseQueries(queries_R, numInputsExecutor, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);


	return queries_R;
}



struct eccPoint **NaorPinkas_OT_Sender_Queries(int writeSocket, int readSocket, int numInputsExecutor)
{
	struct eccPoint **queries_S;
	unsigned char *commBuffer;
	int commBufferLen;


	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	queries_S = deserialiseQueries(commBuffer, numInputsExecutor);
	free(commBuffer);


	return queries_S;
}



void full_NaorPinkas_OT_Sender(int writeSocket, int readSocket, int numInputsExecutor,
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

	/*
	commBufferLen = 0;
	bufferOffset = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	queries_S = deserialiseQueries(commBuffer, numInputsExecutor);
	free(commBuffer);
	*/

	transfers_S = (struct OT_NP_Sender_Transfer **) calloc(numCircuits, sizeof(struct OT_NP_Sender_Transfer *));

	for(i = 0; i < numInputsExecutor; i ++)
	{
		// #pragma omp parallel for private(j, k) schedule(auto)
		for(j = 0; j < numCircuits; j ++)
		{
			k = i * numCircuits + j;

			transfers_S[j] = OT_NP_Transfer(C, queries_S[i], OT_Inputs[0][k], OT_Inputs[1][k], 16, *state, params);

/*			for(h = 0; h < 16; h ++)
			{
				printf("%02X", OT_Inputs[0][k][h]);
			}
			printf("\n");
			for(h = 0; h < 16; h ++)
			{
				printf("%02X", OT_Inputs[1][k][h]);
			}
			printf("\n\n");*/
			
		}

		commBufferLen = 0;
		commBuffer = serialiseTransferStructs(transfers_S, numCircuits, 16, &commBufferLen);
		sendBoth(writeSocket, commBuffer, commBufferLen);
		free(commBuffer);

		for(j = 0; j < numCircuits; j ++)
		{
			clearECC_Point(transfers_S[j] -> a);
			free(transfers_S[j] -> c_0);
			free(transfers_S[j] -> c_1);
			free(transfers_S[j]);
		}
	}

}


unsigned char **full_NaorPinkas_OT_Receiver(int writeSocket, int readSocket, int numInputsExecutor,
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
	/*
	queries_R = (struct OT_NP_Receiver_Query **) calloc(numOTs, sizeof(struct OT_NP_Receiver_Query *));
	
	for(i = 0; i < numInputsExecutor; i ++)
	{
		sigmaBit = permedInputs[i];
		queries_R[i] = OT_NP_query(C, sigmaBit, params, *state);
	}

	commBufferLen = 0;
	commBuffer = serialiseQueries(queries_R, numInputsExecutor, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);
	*/


	outputBytes = (unsigned char **) calloc(numOTs, sizeof(unsigned char *));
	for(i = 0; i < numInputsExecutor; i ++)
	{
		sigmaBit = permedInputs[i];

		commBufferLen = 0;
		commBuffer = receiveBoth(readSocket, commBufferLen);
		transfers_R = deserialiseTransferStructs(commBuffer, numCircuits, 16);
		free(commBuffer);

		// #pragma omp parallel for private(j, k) schedule(auto)
		for(j = 0; j < numCircuits; j ++)
		{
			k = i * numCircuits + j;

			outputBytes[k] = OT_NP_Output_Xb(C, transfers_R[j] -> a, queries_R[i] -> k, transfers_R[j] -> c_0, transfers_R[j] -> c_1, sigmaBit, 16, params);

/*
			for(h = 0; h < 16; h ++)
			{
				printf("%02X", outputBytes[k][h]);
			}
			printf("\n\n");*/


			clearECC_Point(transfers_R[j] -> a);
			free(transfers_R[j] -> c_0);
			free(transfers_R[j] -> c_1);
			free(transfers_R[j]);
		}
	}



	return outputBytes;
}