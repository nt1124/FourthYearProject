unsigned char *serialiseCommitment_C_Array(struct verifierCommitment_ECC **toSerialise, int arrayLen, int *outputLen)
{
	unsigned char *commBuffer;
	int i, bufferOffset = sizeof(int), totalLength = sizeof(int);


	for(i = 0; i < arrayLen; i ++)
	{
		totalLength += sizeOfSerial_ECCPoint(toSerialise[i] -> C_commit);
	}

	commBuffer = (unsigned char *) calloc(totalLength, sizeof(unsigned char));

	memcpy(commBuffer, &arrayLen, sizeof(int));
	for(i = 0; i < arrayLen; i ++)
	{
		serialise_ECC_Point(toSerialise[i] -> C_commit, commBuffer, &bufferOffset);
	}


	*outputLen = bufferOffset;

	return commBuffer;
}


struct verifierCommitment_ECC **deserialiseCommitment_C_Array(unsigned char *commBuffer, int *inputOffset)
{
	struct verifierCommitment_ECC **output;
	int i, bufferOffset = *inputOffset, arrayLen;


	memcpy(&arrayLen, commBuffer + bufferOffset, sizeof(int));
	bufferOffset + sizeof(int);

	output = (struct verifierCommitment_ECC **) calloc(arrayLen, sizeof(struct verifierCommitment_ECC *));

	for(i = 0; i < arrayLen; i ++)
	{
		output[i] = (struct verifierCommitment_ECC *) calloc(1, sizeof(struct verifierCommitment_ECC));
		output[i] -> C_commit = deserialise_ECC_Point(commBuffer, &bufferOffset);
	}


	*inputOffset = bufferOffset;

	return output;
}


unsigned char *serialise_A_B_Array(struct msgOneArrays_ECC **toSerialise,
								int externalArrayLen, int internalArrayLen, int *outputLen)
{
	unsigned char *commBuffer;
	int i, j, bufferOffset = 2*sizeof(int), totalLength = 2*sizeof(int);


	for(i = 0; i < externalArrayLen; i ++)
	{
		for(j = 0; j < internalArrayLen; j ++)
		{
			totalLength += sizeOfSerial_ECCPoint(toSerialise[i] -> A_array[j]);
			totalLength += sizeOfSerial_ECCPoint(toSerialise[i] -> B_array[j]);
		}
	}

	commBuffer = (unsigned char *) calloc(totalLength, sizeof(unsigned char));

	memcpy(commBuffer, &externalArrayLen, sizeof(int));
	memcpy(commBuffer + sizeof(int), &internalArrayLen, sizeof(int));
	for(i = 0; i < externalArrayLen; i ++)
	{
		for(j = 0; j < internalArrayLen; j ++)
		{
			serialise_ECC_Point(toSerialise[i] -> A_array[j], commBuffer, &bufferOffset);
			serialise_ECC_Point(toSerialise[i] -> B_array[j], commBuffer, &bufferOffset);
		}
	}


	*outputLen = bufferOffset;

	return commBuffer;
}


struct msgOneArrays_ECC **deserialise_A_B_Array(unsigned char *commBuffer, int *inputOffset)
{
	struct msgOneArrays_ECC **output;
	int i, j, bufferOffset = *inputOffset;
	int externalArrayLen, internalArrayLen;


	memcpy(&externalArrayLen, commBuffer + bufferOffset, sizeof(int));
	bufferOffset + sizeof(int);
	memcpy(&internalArrayLen, commBuffer + bufferOffset, sizeof(int));
	bufferOffset + sizeof(int);

	output = (struct msgOneArrays_ECC **) calloc(externalArrayLen, sizeof(struct msgOneArrays_ECC *));

	for(i = 0; i < externalArrayLen; i ++)
	{
		output[i] = initMsgOneArray_ECC(internalArrayLen);
		for(j = 0; j < internalArrayLen; j ++)
		{
			output[i] -> A_array[j] = deserialise_ECC_Point(commBuffer, &bufferOffset);
			output[i] -> B_array[j] = deserialise_ECC_Point(commBuffer, &bufferOffset);
		}
	}


	*inputOffset = bufferOffset;

	return output;
}


