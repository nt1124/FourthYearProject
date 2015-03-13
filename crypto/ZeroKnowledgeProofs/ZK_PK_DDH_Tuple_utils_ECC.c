struct verifierCommitment_ECC *initVerifierCommitment_ECC()
{
	struct verifierCommitment_ECC *commitment_box = (struct verifierCommitment_ECC *) calloc(1, sizeof(struct verifierCommitment_ECC));

	mpz_init(commitment_box -> t);
	mpz_init(commitment_box -> c);

	return commitment_box;
}



struct msgOneArrays_ECC *initMsgOneArray_ECC(int stat_SecParam)
{
	struct msgOneArrays_ECC *msgOne = (struct msgOneArrays_ECC *) calloc(1, sizeof(struct msgOneArrays_ECC));
	int i, stat_SecParamDiv2 = stat_SecParam / 2;


	msgOne -> in_I_index = (int *) calloc(stat_SecParamDiv2, sizeof(int));
	msgOne -> roeArray = (mpz_t *) calloc(stat_SecParamDiv2, sizeof(mpz_t));
	msgOne -> A_array = (struct eccPoint**) calloc(stat_SecParam, sizeof(struct eccPoint*));
	msgOne -> B_array = (struct eccPoint**) calloc(stat_SecParam, sizeof(struct eccPoint*));

	msgOne -> notI_Struct = initMsgOne_notI_Array(stat_SecParam);

	for(i = 0; i < stat_SecParamDiv2; i ++)
	{
		mpz_init(msgOne -> roeArray[i]);
	}


	return msgOne;
}


unsigned char *serialisedSecrets_CommitmentBox(struct verifierCommitment_ECC *commitment_box, int *bufferLength)
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


void deserialisedSecrets_CommitmentBox(struct verifierCommitment_ECC *commitment_box, unsigned char *inputBuffer, int *bufferOffset)
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


unsigned char *serialise_A_B_Arrays_ECC(struct msgOneArrays_ECC *toSerialise, int stat_SecParam, int *outputLength)
{
	unsigned char *outputBuffer, *A_Buffer, *B_Buffer;
	int i, outputOffset = 0;

	*outputLength = 0;
	for(i = 0; i < stat_SecParam; i ++)
	{
		*outputLength += sizeOfSerial_ECCPoint(toSerialise -> A_array[i]);
		*outputLength += sizeOfSerial_ECCPoint(toSerialise -> B_array[i]);
	}

	outputBuffer = (unsigned char *) calloc(*outputLength, sizeof(unsigned char));
	
	
	for(i = 0; i < stat_SecParam; i ++)
	{
		serialise_ECC_Point(toSerialise -> A_array[i], outputBuffer, &outputOffset);
		serialise_ECC_Point(toSerialise -> B_array[i], outputBuffer, &outputOffset);
	}


	return outputBuffer;
}


void deserialise_A_B_Arrays_ECC(struct msgOneArrays_ECC *outputStruct, unsigned char *inputBuffer, int *bufferOffset, int stat_SecParam)
{
	int localOffset = *bufferOffset, i;

	
	for(i = 0; i < stat_SecParam; i ++)
	{
		outputStruct -> A_array[i] = deserialise_ECC_Point(inputBuffer, &localOffset);
		outputStruct -> B_array[i] = deserialise_ECC_Point(inputBuffer, &localOffset);
	}


	*bufferOffset = localOffset;
}


unsigned char *serialise_Array_A_B_Arrays_ECC(struct msgOneArrays_ECC **toSerialise, int numTuples, int stat_SecParam, int *outputLength)
{
	unsigned char *outputBuffer, *A_Buffer, *B_Buffer;
	int i, j, outputOffset = 0;

	*outputLength = 0;

	for(j = 0; j < numTuples; j ++)
	{
		for(i = 0; i < stat_SecParam; i ++)
		{
			*outputLength += sizeOfSerial_ECCPoint(toSerialise[j] -> A_array[i]);
			*outputLength += sizeOfSerial_ECCPoint(toSerialise[j] -> B_array[i]);
		}
	}


	outputBuffer = (unsigned char *) calloc(*outputLength, sizeof(unsigned char));
	
	
	for(j = 0; j < numTuples; j ++)
	{
		for(i = 0; i < stat_SecParam; i ++)
		{
			serialise_ECC_Point(toSerialise[j] -> A_array[i], outputBuffer, &outputOffset);
			serialise_ECC_Point(toSerialise[j] -> B_array[i], outputBuffer, &outputOffset);
		}
	}


	return outputBuffer;
}


void deserialise_Array_A_B_Arrays_ECC(struct msgOneArrays_ECC **outputStruct, int numTuples, unsigned char *inputBuffer, int *bufferOffset, int stat_SecParam)
{
	int localOffset = *bufferOffset, i, j;

	
	for(j = 0; j < numTuples; j ++)
	{
		for(i = 0; i < stat_SecParam; i ++)
		{
			outputStruct[j] -> A_array[i] = deserialise_ECC_Point(inputBuffer, &localOffset);
			outputStruct[j] -> B_array[i] = deserialise_ECC_Point(inputBuffer, &localOffset);
		}
	}


	*bufferOffset = localOffset;
}