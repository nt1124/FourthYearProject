struct OT_NP_Receiver_Query *init_OT_NP_Receiver_Query()
{
	struct OT_NP_Receiver_Query *toReturn;


	toReturn = (struct OT_NP_Receiver_Query *) calloc(1, sizeof(struct OT_NP_Receiver_Query));
	mpz_init(toReturn -> k);


	return toReturn;
}


struct OT_NP_Sender_Transfer *init_OT_NP_Sender_Transfer()
{
	struct OT_NP_Sender_Transfer *toReturn;


	toReturn = (struct OT_NP_Sender_Transfer *) calloc(1, sizeof(struct OT_NP_Sender_Transfer));


	return toReturn;
}



unsigned char *serialiseQueries(struct OT_NP_Receiver_Query **queries, int numQueries, int *outputLength)
{
	unsigned char *outputBuffer;
	int i, localLength = 0, localOffset = 0;

	for(i = 0; i < numQueries; i ++)
	{
		localLength += sizeOfSerial_ECCPoint(queries[i] -> h);
	}

	outputBuffer = (unsigned char *) calloc(localLength, sizeof(unsigned char));

	for(i = 0; i < numQueries; i ++)
	{
		serialise_ECC_Point(queries[i] -> h, outputBuffer, &localOffset);
	}

	*outputLength = localOffset;

	return outputBuffer;
}


struct eccPoint **deserialiseQueries(unsigned char *inputBuffer, int numQueries)
{
	struct eccPoint **outputQueries = (struct eccPoint **) calloc(numQueries, sizeof(struct eccPoint*));
	int i, localOffset = 0;


	for(i = 0; i < numQueries; i ++)
	{
		outputQueries[i] = deserialise_ECC_Point(inputBuffer, &localOffset);
	}


	return outputQueries;
}


unsigned char *serialiseTransferStructs(struct OT_NP_Sender_Transfer **transferStructs, int numTransfers, int msgLength, int *outputLength)
{
	unsigned char *outputBuffer, *tempBuffer;
	int i, localLength = 0, localOffset = 0;


	for(i = 0; i < numTransfers; i ++)
	{
		localLength += sizeOfSerial_ECCPoint(transferStructs[i] -> a);
		localLength += 2 * msgLength;
	}

	outputBuffer = (unsigned char *) calloc(localLength, sizeof(unsigned char));

	for(i = 0; i < numTransfers; i ++)
	{
		serialise_ECC_Point(transferStructs[i] -> a, outputBuffer, &localOffset);

		memcpy(outputBuffer + localOffset, transferStructs[i] -> c_0, msgLength);
		localOffset += msgLength;

		memcpy(outputBuffer + localOffset, transferStructs[i] -> c_1, msgLength);
		localOffset += msgLength;
	}


	*outputLength = localOffset;

	return outputBuffer;
}


struct OT_NP_Sender_Transfer **deserialiseTransferStructs(unsigned char *inputBuffer, int numTransfers, int msgLength)
{
	struct OT_NP_Sender_Transfer **outputStructs;
	int i, localOffset = 0;


	outputStructs = (struct OT_NP_Sender_Transfer **) calloc(numTransfers, sizeof(struct OT_NP_Sender_Transfer *));

	for(i = 0; i < numTransfers; i ++)
	{
		outputStructs[i] = (struct OT_NP_Sender_Transfer *) calloc(1, sizeof(struct OT_NP_Sender_Transfer));

		outputStructs[i] -> a = deserialise_ECC_Point(inputBuffer, &localOffset);

		outputStructs[i] -> c_0 = (unsigned char *) calloc(msgLength, sizeof(unsigned char));
		memcpy(outputStructs[i] -> c_0, inputBuffer + localOffset, msgLength);
		localOffset += msgLength;


		outputStructs[i] -> c_1 = (unsigned char *) calloc(msgLength, sizeof(unsigned char));
		memcpy(outputStructs[i] -> c_1, inputBuffer + localOffset, msgLength);
		localOffset += msgLength;
	}


	return outputStructs;
}