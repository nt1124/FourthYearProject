struct witnessStruct *initWitnessStruc(int length)
{
	struct witnessStruct *toReturn = (struct witnessStruct *) calloc(1, sizeof(struct witnessStruct));
	int i;


	toReturn -> IDs = (int*) calloc(length, sizeof(int));
	toReturn -> witnesses = (mpz_t*) calloc(length, sizeof(mpz_t));

	for(i = 0; i < length; i ++)
	{
		mpz_init(toReturn -> witnesses[i]);
	}


	return toReturn;
}


struct verifierCommitment *initVerifierCommitment()
{
	struct verifierCommitment *commitment_box = (struct verifierCommitment *) calloc(1, sizeof(struct verifierCommitment));

	mpz_init(commitment_box -> t);
	mpz_init(commitment_box -> c);
	mpz_init(commitment_box -> C_commit);

	return commitment_box;
}


struct msgOne_notI_Arrays *initMsgOne_notI_Array(int stat_SecParam)
{
	struct msgOne_notI_Arrays *msg_notI_One = (struct msgOne_notI_Arrays *) calloc(1, sizeof(struct msgOne_notI_Arrays));
	int i, stat_SecParamDiv2 = stat_SecParam / 2;


	msg_notI_One -> not_in_I_index = (int *) calloc(stat_SecParamDiv2, sizeof(int));
	msg_notI_One -> Z_array = (mpz_t *) calloc(stat_SecParamDiv2, sizeof(mpz_t));
	msg_notI_One -> C_array = (mpz_t *) calloc(stat_SecParamDiv2, sizeof(mpz_t));

	for(i = 0; i < stat_SecParamDiv2; i ++)
	{
		mpz_init(msg_notI_One -> Z_array[i]);
		mpz_init(msg_notI_One -> C_array[i]);
	}

	return msg_notI_One;
}


struct msgOneArrays *initMsgOneArray(int stat_SecParam)
{
	struct msgOneArrays *msgOne = (struct msgOneArrays *) calloc(1, sizeof(struct msgOneArrays));
	int i, stat_SecParamDiv2 = stat_SecParam / 2;


	msgOne -> in_I_index = (int *) calloc(stat_SecParamDiv2, sizeof(int));
	msgOne -> roeArray = (mpz_t *) calloc(stat_SecParamDiv2, sizeof(mpz_t));
	msgOne -> A_array = (mpz_t*) calloc(stat_SecParam, sizeof(mpz_t));
	msgOne -> B_array = (mpz_t*) calloc(stat_SecParam, sizeof(mpz_t));

	msgOne -> notI_Struct = initMsgOne_notI_Array(stat_SecParam);

	for(i = 0; i < stat_SecParamDiv2; i ++)
	{
		mpz_init(msgOne -> roeArray[i]);
		mpz_init(msgOne -> A_array[i]);
		mpz_init(msgOne -> B_array[i]);
	}
	for(i = stat_SecParamDiv2; i < stat_SecParam; i ++)
	{
		mpz_init(msgOne -> A_array[i]);
		mpz_init(msgOne -> B_array[i]);
	}


	return msgOne;
}


unsigned char *serialisedSecrets_CommitmentBox(struct verifierCommitment *commitment_box, int *bufferLength)
{
	unsigned char *outputBuffer;
	int totalLength = 2 * sizeof(int);
	int offset = 0;


	totalLength += sizeof(mp_limb_t) * mpz_size(commitment_box -> c);
	totalLength += sizeof(mp_limb_t) * mpz_size(commitment_box -> t);


	outputBuffer = (unsigned char *) calloc(totalLength, sizeof(unsigned char));
	serialiseMPZ(commitment_box -> c, outputBuffer, &offset);
	serialiseMPZ(commitment_box -> t, outputBuffer, &offset);


	*bufferLength = offset;

	return outputBuffer;
}


void deserialisedSecrets_CommitmentBox(struct verifierCommitment *commitment_box, unsigned char *inputBuffer, int *bufferOffset)
{
	mpz_t *temp;
	int localOffset = *bufferOffset;

	temp = deserialiseMPZ(inputBuffer, &localOffset);
	mpz_set(commitment_box -> c, *temp);

	temp = deserialiseMPZ(inputBuffer, &localOffset);
	mpz_set(commitment_box -> t, *temp);
	free(temp);

	*bufferOffset = localOffset;
}


unsigned char *serialise_A_B_Arrays(struct msgOneArrays *toSerialise, int stat_SecParam, int *outputLength)
{
	unsigned char *outputBuffer, *A_Buffer, *B_Buffer;
	int A_Length = 0, B_Length = 0;

	A_Buffer = serialiseMPZ_Array(toSerialise -> A_array, stat_SecParam, &A_Length);
	B_Buffer = serialiseMPZ_Array(toSerialise -> B_array, stat_SecParam, &B_Length);

	*outputLength = A_Length + B_Length;
	outputBuffer = (unsigned char *) calloc(*outputLength, sizeof(unsigned char));

	memcpy(outputBuffer, A_Buffer, A_Length);
	memcpy(outputBuffer + A_Length, B_Buffer, B_Length);


	return outputBuffer;
}


void deserialise_A_B_Arrays(struct msgOneArrays *outputStruct, unsigned char *inputBuffer, int *bufferOffset)
{
	int localOffset = *bufferOffset;

	outputStruct -> A_array = deserialiseMPZ_Array(inputBuffer, &localOffset);
	outputStruct -> B_array = deserialiseMPZ_Array(inputBuffer, &localOffset);

	*bufferOffset = localOffset;
}



