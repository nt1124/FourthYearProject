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


unsigned char *serialiseVerifierQueries(struct verifierCommitment_ECC **commitment_boxes, int arrayLen, int *outputLen)
{
	unsigned char *commBuffer;
	int i, totalLength = sizeof(int), bufferOffset = sizeof(int);


	for(i = 0; i < arrayLen; i ++)
	{
		totalLength += sizeof(mp_limb_t) * ( mpz_size(commitment_boxes[i] -> c) + mpz_size(commitment_boxes[i] -> t) );
	}

	commBuffer = (unsigned char *) calloc(totalLength, sizeof(unsigned char));

	memcpy(commBuffer, &arrayLen, sizeof(int));
	for(i = 0; i < arrayLen; i ++)
	{
		serialiseMPZ(commitment_boxes[i] -> c, commBuffer, &bufferOffset);
		serialiseMPZ(commitment_boxes[i] -> t, commBuffer, &bufferOffset);
	}


	*outputLen = bufferOffset;

	return commBuffer;
}


void deserialiseVerifierQueries(struct verifierCommitment_ECC **commitment_boxes, unsigned char *commBuffer, int arrayLen, int *outputLen)
{
	int i, bufferOffset = *outputLen;
	mpz_t *tempMPZ;


	memcpy(&arrayLen, commBuffer + bufferOffset, sizeof(int));
	bufferOffset += sizeof(int);

	for(i = 0; i < arrayLen; i ++)
	{
		tempMPZ = deserialiseMPZ(commBuffer, &bufferOffset);
		mpz_init_set(commitment_boxes[i] -> c, *tempMPZ);
		mpz_clear(*tempMPZ);
		free(tempMPZ);

		deserialiseMPZ(commBuffer, &bufferOffset);
		mpz_init_set(commitment_boxes[i] -> t, *tempMPZ);
		mpz_clear(*tempMPZ);
		free(tempMPZ);
	}


	*outputLen = bufferOffset;
}


mpz_t *computeProverTwo_ECC_1Of2(struct eccParams *params, unsigned char *J_set,
								struct msgOneArrays_ECC *msgArray, struct witnessStruct *witnessesArray,
								mpz_t *cShares, struct alphaAndA_Struct *alphaAndA)
{
	mpz_t temp1, temp2, *z_ToSend, *temp;
	int i, j_in_I = 0, j_not_I = 0;
	int bufferOffset = sizeof(int);


	mpz_init(temp1);
	mpz_init(temp2);


	z_ToSend = (mpz_t*) calloc(2, sizeof(mpz_t));

	for(i = 0; i < 2; i ++)
	{
		mpz_init(z_ToSend[i]);

		// i IS in I = J^{bar}
		if(0x00 == J_set[i])
		{
			mpz_set(z_ToSend[i], msgArray -> notI_Struct -> Z_array[j_not_I]);

			j_not_I ++;
		}
		else
		{
			// z_i = c_i * w_i + ρ_i
			mpz_mul(temp1, cShares[i], witnessesArray -> witnesses[i]);
			mpz_add(temp2, temp1, msgArray -> roeArray[j_in_I]);
			mpz_mod(z_ToSend[i], temp2, params -> n);

			j_in_I ++;
		}
	}


	return z_ToSend;
}


mpz_t **computeAllProverTwo_ECC_1Of2(struct eccParams *params, unsigned char **J_set,
								struct msgOneArrays_ECC **msgArray, struct witnessStruct **witnessesArray,
								mpz_t **cShares, int arrayLen,
								struct alphaAndA_Struct **alphaAndA)
{
	mpz_t **z_ToSend = (mpz_t **) calloc(arrayLen, sizeof(mpz_t*));
	int i;


	for(i = 0; i < arrayLen; i ++)
	{
		z_ToSend[i] = computeProverTwo_ECC_1Of2(params, J_set[i], msgArray[i], witnessesArray[i],
												cShares[i], alphaAndA[i]);
	}

	return z_ToSend;
}



unsigned char *serialiseProverTwo_ECC_1Of2(mpz_t **z_ToSend, mpz_t **cShares, struct alphaAndA_Struct **alphaAndA,
										int arrayLen, int *outputLen)
{
	unsigned char *commBuffer;
	int i, j;
	int bufferOffset = sizeof(int);
	int totalLength = sizeof(int);



	for(i = 0; i < arrayLen; i ++)
	{
		totalLength += sizeof(mp_limb_t) * (mpz_size(z_ToSend[i][0]) + mpz_size(z_ToSend[i][1]));
		totalLength += sizeof(mp_limb_t) * (mpz_size(cShares[i][0]) + mpz_size(cShares[i][1]));
		totalLength += (mpz_size(alphaAndA[i] -> a) * sizeof(mp_limb_t));
		totalLength += (5 * sizeof(int));
	}


	commBuffer = (unsigned char *) calloc(totalLength, sizeof(unsigned char));

	memcpy(commBuffer, &arrayLen, sizeof(int));
	for(i = 0; i < arrayLen; i ++)
	{
		serialiseMPZ(z_ToSend[i][0], commBuffer, &bufferOffset);
		serialiseMPZ(z_ToSend[i][1], commBuffer, &bufferOffset);

		serialiseMPZ(cShares[i][0], commBuffer, &bufferOffset);
		serialiseMPZ(cShares[i][1], commBuffer, &bufferOffset);

		serialiseMPZ(alphaAndA[i] -> a, commBuffer, &bufferOffset);
	}


	*outputLen = bufferOffset;

	return commBuffer;
}



unsigned char *deserialiseProverTwo_ECC_1Of2(mpz_t **z_ToSend, mpz_t **cShares, struct alphaAndA_Struct **alphaAndA,
										int arrayLen, int *outputLen)
{
	unsigned char *commBuffer;
	int i, j;
	int bufferOffset = sizeof(int);
	int totalLength = sizeof(int);



	for(i = 0; i < arrayLen; i ++)
	{
		totalLength += sizeof(mp_limb_t) * (mpz_size(z_ToSend[i][0]) + mpz_size(z_ToSend[i][1]));
		totalLength += sizeof(mp_limb_t) * (mpz_size(cShares[i][0]) + mpz_size(cShares[i][1]));
		totalLength += (mpz_size(alphaAndA[i] -> a) * sizeof(mp_limb_t));
		totalLength += (5 * sizeof(int));
	}


	commBuffer = (unsigned char *) calloc(totalLength, sizeof(unsigned char));

	memcpy(commBuffer, &arrayLen, sizeof(int));
	for(i = 0; i < arrayLen; i ++)
	{
		serialiseMPZ(z_ToSend[i][0], commBuffer, &bufferOffset);
		serialiseMPZ(z_ToSend[i][1], commBuffer, &bufferOffset);

		serialiseMPZ(cShares[i][0], commBuffer, &bufferOffset);
		serialiseMPZ(cShares[i][1], commBuffer, &bufferOffset);

		serialiseMPZ(alphaAndA[i] -> a, commBuffer, &bufferOffset);
	}


	*outputLen = bufferOffset;

	return commBuffer;
}