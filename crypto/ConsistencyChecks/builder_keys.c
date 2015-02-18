struct secret_builderPRS_Keys *init_secret_input_keys(int numInputs, int stat_SecParam)
{
	struct secret_builderPRS_Keys *toReturn = (struct secret_builderPRS_Keys*) calloc(1, sizeof(struct secret_builderPRS_Keys));
	int i;

	toReturn -> numKeyPairs = numInputs;
	toReturn -> secret_keyPairs = (mpz_t **) calloc(numInputs, sizeof(mpz_t*));

	for(i = 0; i < numInputs; i ++)
	{
		toReturn -> secret_keyPairs[i] = (mpz_t *) calloc(2, sizeof(mpz_t));

		mpz_init(toReturn -> secret_keyPairs[i][0]);
		mpz_init(toReturn -> secret_keyPairs[i][1]);
	}

	toReturn -> stat_SecParam = stat_SecParam;
	toReturn -> secret_circuitKeys = (mpz_t *) calloc(stat_SecParam, sizeof(mpz_t));

	for(i = 0; i < stat_SecParam; i ++)
	{
		mpz_init(toReturn -> secret_circuitKeys[i]);
	}

	return toReturn;
}


struct public_builderPRS_Keys *init_public_input_keys(int numInputs, int stat_SecParam)
{
	struct public_builderPRS_Keys *toReturn = (struct public_builderPRS_Keys*) calloc(1, sizeof(struct public_builderPRS_Keys));
	int i;

	toReturn -> numKeyPairs = numInputs;
	toReturn -> public_keyPairs = (mpz_t **) calloc(numInputs, sizeof(mpz_t*));

	for(i = 0; i < numInputs; i ++)
	{
		toReturn -> public_keyPairs[i] = (mpz_t *) calloc(2, sizeof(mpz_t));

		mpz_init(toReturn -> public_keyPairs[i][0]);
		mpz_init(toReturn -> public_keyPairs[i][1]);
	}

	toReturn -> stat_SecParam = stat_SecParam;
	toReturn -> public_circuitKeys = (mpz_t *) calloc(stat_SecParam, sizeof(mpz_t));

	for(i = 0; i < stat_SecParam; i ++)
	{
		mpz_init(toReturn -> public_circuitKeys[i]);
	}

	return toReturn;
}



void free_secret_key_set(struct secret_builderPRS_Keys *toFree)
{
	int i;

	for(i = 0; i < toFree -> numKeyPairs; i ++)
	{
		free(toFree -> secret_keyPairs[i]);
	}

	free(toFree -> secret_circuitKeys);
}

void free_public_key_set(struct public_builderPRS_Keys *toFree)
{
	int i;

	for(i = 0; i < toFree -> numKeyPairs; i ++)
	{
		free(toFree -> public_keyPairs[i]);
	}

	free(toFree -> public_circuitKeys);
}



struct secret_builderPRS_Keys *generateSecrets(int numInputs, int stat_SecParam, struct DDH_Group *group, gmp_randstate_t state)
{
	struct secret_builderPRS_Keys *secret_inputs;
	int i;
	
	secret_inputs = init_secret_input_keys(numInputs, stat_SecParam);

	for(i = 0; i < secret_inputs -> numKeyPairs; i ++)
	{
		mpz_urandomm(secret_inputs -> secret_keyPairs[i][0], state, group -> q);
		mpz_urandomm(secret_inputs -> secret_keyPairs[i][1], state, group -> q);
	}

	for(i = 0; i < secret_inputs -> stat_SecParam; i ++)
	{
		mpz_urandomm(secret_inputs -> secret_circuitKeys[i], state, group -> q);
	}

	return secret_inputs;
}


struct public_builderPRS_Keys *computePublicInputs(struct secret_builderPRS_Keys *secret_inputs, struct DDH_Group *group)
{
	struct public_builderPRS_Keys *public_inputs;
	int i;
	
	public_inputs = init_public_input_keys(secret_inputs -> numKeyPairs, secret_inputs -> stat_SecParam);

	for(i = 0; i < secret_inputs -> numKeyPairs; i ++)
	{
		mpz_powm(public_inputs -> public_keyPairs[i][0], group -> g, secret_inputs -> secret_keyPairs[i][0], group -> p);
		mpz_powm(public_inputs -> public_keyPairs[i][1], group -> g, secret_inputs -> secret_keyPairs[i][1], group -> p);
	}

	for(i = 0; i < secret_inputs -> stat_SecParam; i ++)
	{
		mpz_powm(public_inputs -> public_circuitKeys[i], group -> g, secret_inputs -> secret_circuitKeys[i], group -> p);
	}

	return public_inputs;
}


unsigned char *compute_Key_b_Input_i_Circuit_j(struct secret_builderPRS_Keys *secret_inputs, struct public_builderPRS_Keys *public_inputs, struct DDH_Group *group,
										int i, int j, unsigned char inputBit, unsigned char permutation)
{
	mpz_t mpz_representation;
	unsigned char *rawBytes, *hashedBytes, *halfHash = (unsigned char *) calloc(17, sizeof(unsigned char));
	int outputLength = 0;

	mpz_init(mpz_representation);


	mpz_powm(mpz_representation, public_inputs -> public_keyPairs[i][inputBit], secret_inputs -> secret_circuitKeys[j], group -> p);

	rawBytes = convertMPZToBytes(mpz_representation, &outputLength);

	hashedBytes = sha256_full(rawBytes, outputLength);

	memcpy(halfHash, rawBytes, 16);
	halfHash[16] = (0x01 & permutation) ^ inputBit;

	free(hashedBytes);
	free(rawBytes);

	return halfHash;
}



unsigned char *compute_Key_b_Input_i_Circuit_j(mpz_t secret_input, struct public_builderPRS_Keys *public_inputs, struct DDH_Group *group,
										int i, unsigned char inputBit)
{
	mpz_t mpz_representation;
	unsigned char *rawBytes, *hashedBytes, *halfHash = (unsigned char *) calloc(16, sizeof(unsigned char));
	int outputLength = 0;

	mpz_init(mpz_representation);


	mpz_powm(mpz_representation, public_inputs -> public_keyPairs[i][inputBit], secret_input, group -> p);

	rawBytes = convertMPZToBytes(mpz_representation, &outputLength);
	hashedBytes = sha256_full(rawBytes, outputLength);

	memcpy(halfHash, rawBytes, 16);

	free(hashedBytes);
	free(rawBytes);

	return halfHash;
}





unsigned char *serialisePublicInputs(struct public_builderPRS_Keys *public_inputs, int *outputLength)
{
	unsigned char *outputBuffer;
	int totalLength = 0, i, tempNumKeyPairs, tempStat_SecParam;
	int outputOffset = sizeof(int);


	for(i = 0; i < public_inputs -> numKeyPairs; i ++)
	{
		totalLength += ( sizeof(mp_limb_t) * mpz_size(public_inputs -> public_keyPairs[i][0]) );
		totalLength += ( sizeof(mp_limb_t) * mpz_size(public_inputs -> public_keyPairs[i][1]) );
	}
	for(i = 0; i < public_inputs -> stat_SecParam; i ++)
	{
		totalLength += ( sizeof(mp_limb_t) * mpz_size(public_inputs -> public_circuitKeys[i]) );
	}
	totalLength += (2 + public_inputs -> stat_SecParam + 2 * public_inputs -> numKeyPairs) * sizeof(int);

	tempNumKeyPairs = public_inputs -> numKeyPairs;
	tempStat_SecParam = public_inputs -> stat_SecParam;

	outputBuffer = (unsigned char*) calloc(totalLength, sizeof(unsigned char));
	memcpy(outputBuffer, &tempNumKeyPairs, sizeof(int));
	memcpy(outputBuffer + outputOffset, &tempStat_SecParam, sizeof(int));
	outputOffset += sizeof(int);


	for(i = 0; i < public_inputs -> numKeyPairs; i ++)
	{
		serialiseMPZ(public_inputs -> public_keyPairs[i][0], outputBuffer, &outputOffset);
		serialiseMPZ(public_inputs -> public_keyPairs[i][1], outputBuffer, &outputOffset);
	}
	for(i = 0; i < public_inputs -> stat_SecParam; i ++)
	{
		serialiseMPZ(public_inputs -> public_circuitKeys[i], outputBuffer, &outputOffset);
	}

	*outputLength = outputOffset;

	return outputBuffer;
}


struct public_builderPRS_Keys *deserialisePublicInputs(unsigned char *inputBuffer, int *bufferOffset)
{
	struct public_builderPRS_Keys *toReturn;
	int i, inputOffset = *bufferOffset, numKeyPairs, stat_SecParam;
	mpz_t *tempMPZ;


	memcpy(&numKeyPairs, inputBuffer + inputOffset, sizeof(int));
	inputOffset += sizeof(int);
	memcpy(&stat_SecParam, inputBuffer + inputOffset, sizeof(int));
	inputOffset += sizeof(int);

	toReturn = init_public_input_keys(numKeyPairs, stat_SecParam);


	for(i = 0; i < toReturn -> numKeyPairs; i ++)
	{
		tempMPZ = deserialiseMPZ(inputBuffer, &inputOffset);
		mpz_set(toReturn -> public_keyPairs[i][0], *tempMPZ);
		free(tempMPZ);

		tempMPZ = deserialiseMPZ(inputBuffer, &inputOffset);
		mpz_set(toReturn -> public_keyPairs[i][1], *tempMPZ);
		free(tempMPZ);
	}

	for(i = 0; i < toReturn -> stat_SecParam; i ++)
	{
		tempMPZ = deserialiseMPZ(inputBuffer, &inputOffset);
		mpz_set(toReturn -> public_circuitKeys[i], *tempMPZ);
		free(tempMPZ);
	}

	*bufferOffset = inputOffset;

	return toReturn;
}


unsigned char *serialise_Requested_CircuitSecrets(struct secret_builderPRS_Keys *secret_inputs,
												unsigned char *J_set, int *outputLength)
{
	unsigned char *outputBuffer;
	int totalLength = 0, outputOffset = 0;
	int i = 0;


	for(i = 0; i < secret_inputs -> stat_SecParam; i ++)
	{
		if(0x01 == J_set[i])
		{
			totalLength += ( sizeof(mp_limb_t) * mpz_size(secret_inputs -> secret_circuitKeys[i]) );		
		}
	}

	outputBuffer = (unsigned char *) calloc(totalLength, sizeof(unsigned char));

	for(i = 0; i < secret_inputs -> stat_SecParam; i ++)
	{
		if(0x01 == J_set[i])
		{
			serialiseMPZ(secret_inputs -> secret_circuitKeys[i], outputBuffer, &outputOffset);
		}
	}

	*outputLength = outputOffset;

	return outputBuffer;
}


mpz_t **deserialise_Requested_CircuitSecrets(unsigned char *inputBuffer, int stat_SecParam,
											unsigned char *J_set,
											struct public_builderPRS_Keys *public_input,
											struct DDH_Group *group)
{
	int inputOffset = 0, j = 0;

	mpz_t **outputArray = (mpz_t**) calloc(stat_SecParam, sizeof(mpz_t*));
	mpz_t temp;

	mpz_init(temp);


	for(j = 0; j < stat_SecParam; j ++)
	{
		if(0x01 == J_set[j])
		{
			outputArray[j] = deserialiseMPZ(inputBuffer, &inputOffset);
			mpz_powm(temp, group -> g, *(outputArray[j]), group -> p);
			if(0 != mpz_cmp(temp, public_input -> public_circuitKeys[j]))
			{
				free(outputArray);
				return NULL;
			}
		}
		else
		{
			outputArray[j] = NULL;
		}
	}

	return outputArray;
}







