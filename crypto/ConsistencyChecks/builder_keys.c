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


struct secret_builderPRS_Keys *init_check_comp_secrets(int numInputs, int stat_SecParam, struct secret_builderPRS_Keys *mainSecrets)
{
	struct secret_builderPRS_Keys *toReturn = (struct secret_builderPRS_Keys*) calloc(1, sizeof(struct secret_builderPRS_Keys));
	int i;

	toReturn -> numKeyPairs = numInputs;
	toReturn -> secret_keyPairs = (mpz_t **) calloc(numInputs, sizeof(mpz_t*));

	for(i = 0; i < numInputs; i ++)
	{
		toReturn -> secret_keyPairs[i] = (mpz_t *) calloc(2, sizeof(mpz_t));

		mpz_init_set(toReturn -> secret_keyPairs[i][0], mainSecrets -> secret_keyPairs[i][0]);
		mpz_init_set(toReturn -> secret_keyPairs[i][1], mainSecrets -> secret_keyPairs[i][1]);
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
	toReturn -> public_keyPairs = (struct eccPoint ***) calloc(numInputs, sizeof(struct eccPoint**));

	for(i = 0; i < numInputs; i ++)
	{
		toReturn -> public_keyPairs[i] = (struct eccPoint **) calloc(2, sizeof(struct eccPoint*));
	}

	toReturn -> stat_SecParam = stat_SecParam;
	toReturn -> public_circuitKeys = (struct eccPoint **) calloc(stat_SecParam, sizeof(struct eccPoint*));


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
		clearECC_Point(toFree -> public_keyPairs[i][0]);
		clearECC_Point(toFree -> public_keyPairs[i][1]);
		free(toFree -> public_keyPairs[i]);
	}


	free(toFree -> public_circuitKeys);
}



struct secret_builderPRS_Keys *generateSecrets(int numInputs, int stat_SecParam, struct eccParams *params, gmp_randstate_t state)
{
	struct secret_builderPRS_Keys *secret_inputs;
	int i;
	

	secret_inputs = init_secret_input_keys(numInputs, stat_SecParam);

	for(i = 0; i < secret_inputs -> numKeyPairs; i ++)
	{
		mpz_urandomm(secret_inputs -> secret_keyPairs[i][0], state, params -> n);
		mpz_urandomm(secret_inputs -> secret_keyPairs[i][1], state, params -> n);
	}

	for(i = 0; i < secret_inputs -> stat_SecParam; i ++)
	{
		mpz_urandomm(secret_inputs -> secret_circuitKeys[i], state, params -> n);
	}


	return secret_inputs;
}


// Generate secrets 
struct secret_builderPRS_Keys *generateSecretsCheckComp(int numInputs, int check_stat_SecParam, struct secret_builderPRS_Keys *mainSecrets,
														struct eccParams *params, gmp_randstate_t state)
{
	struct secret_builderPRS_Keys *secret_inputs;
	int i;
	

	secret_inputs = init_check_comp_secrets(numInputs, check_stat_SecParam, mainSecrets);

	for(i = 0; i < secret_inputs -> stat_SecParam; i ++)
	{
		mpz_urandomm(secret_inputs -> secret_circuitKeys[i], state, params -> n);
	}


	return secret_inputs;
}


struct public_builderPRS_Keys *computePublicInputs(struct secret_builderPRS_Keys *secret_inputs, struct eccParams *params)
{
	struct public_builderPRS_Keys *public_inputs;
	int i;


	public_inputs = init_public_input_keys(secret_inputs -> numKeyPairs, secret_inputs -> stat_SecParam);

	for(i = 0; i < secret_inputs -> numKeyPairs; i ++)
	{
		public_inputs -> public_keyPairs[i][0] = windowedScalarPoint(secret_inputs -> secret_keyPairs[i][0], params -> g, params);
		public_inputs -> public_keyPairs[i][1] = windowedScalarPoint(secret_inputs -> secret_keyPairs[i][1], params -> g, params);
	}

	for(i = 0; i < secret_inputs -> stat_SecParam; i ++)
	{
		public_inputs -> public_circuitKeys[i] = windowedScalarPoint(secret_inputs -> secret_circuitKeys[i], params -> g, params);
	}

	return public_inputs;
}


unsigned char *compute_Key_b_Input_i_Circuit_j(struct secret_builderPRS_Keys *secret_inputs, struct public_builderPRS_Keys *public_inputs, struct eccParams *params,
										int i, int j, unsigned char inputBit, unsigned char permutation)
{
	struct eccPoint *pointRep;
	unsigned char *rawBytes, *hashedBytes, *halfHash = (unsigned char *) calloc(16, sizeof(unsigned char));
	int outputLength = 0;


	pointRep = windowedScalarPoint(secret_inputs -> secret_circuitKeys[j], public_inputs -> public_keyPairs[i][inputBit], params);

	rawBytes = convertMPZToBytes(pointRep -> x, &outputLength);
	hashedBytes = sha256_full(rawBytes, outputLength);

	memcpy(halfHash, hashedBytes, 16);


	clearECC_Point(pointRep);
	free(hashedBytes);
	free(rawBytes);

	return halfHash;
}

unsigned char *compute_Key_b_Input_i_Circuit_j(mpz_t secret_input, struct public_builderPRS_Keys *public_inputs, struct eccParams *params,
										int i, unsigned char inputBit)
{
	struct eccPoint *pointRep;
	unsigned char *rawBytes, *hashedBytes, *halfHash = (unsigned char *) calloc(16, sizeof(unsigned char));
	int outputLength = 0;


	pointRep = windowedScalarPoint(secret_input, public_inputs -> public_keyPairs[i][inputBit], params);

	rawBytes = convertMPZToBytes(pointRep -> x, &outputLength);
	hashedBytes = sha256_full(rawBytes, outputLength);

	memcpy(halfHash, hashedBytes, 16);


	clearECC_Point(pointRep);
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
		totalLength += sizeOfSerial_ECCPoint(public_inputs -> public_keyPairs[i][0]);
		totalLength += sizeOfSerial_ECCPoint(public_inputs -> public_keyPairs[i][1]);
	}
	for(i = 0; i < public_inputs -> stat_SecParam; i ++)
	{
		totalLength += sizeOfSerial_ECCPoint(public_inputs -> public_circuitKeys[i]);
	}

	totalLength += (2 * sizeof(int));

	tempNumKeyPairs = public_inputs -> numKeyPairs;
	tempStat_SecParam = public_inputs -> stat_SecParam;

	outputBuffer = (unsigned char*) calloc(totalLength, sizeof(unsigned char));
	memcpy(outputBuffer, &tempNumKeyPairs, sizeof(int));
	memcpy(outputBuffer + outputOffset, &tempStat_SecParam, sizeof(int));
	outputOffset += sizeof(int);


	for(i = 0; i < public_inputs -> numKeyPairs; i ++)
	{
		serialise_ECC_Point(public_inputs -> public_keyPairs[i][0], outputBuffer, &outputOffset);
		serialise_ECC_Point(public_inputs -> public_keyPairs[i][1], outputBuffer, &outputOffset);
	}
	for(i = 0; i < public_inputs -> stat_SecParam; i ++)
	{
		serialise_ECC_Point(public_inputs -> public_circuitKeys[i], outputBuffer, &outputOffset);
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
		toReturn -> public_keyPairs[i][0] = deserialise_ECC_Point(inputBuffer, &inputOffset);
		toReturn -> public_keyPairs[i][1] = deserialise_ECC_Point(inputBuffer, &inputOffset);
	}

	for(i = 0; i < toReturn -> stat_SecParam; i ++)
	{
		toReturn -> public_circuitKeys[i] = deserialise_ECC_Point(inputBuffer, &inputOffset);
	}

	*bufferOffset = inputOffset;

	return toReturn;
}


unsigned char *serialise_Requested_CircuitSecrets(struct secret_builderPRS_Keys *secret_inputs, unsigned int *seedList,
												ub4 **circuitSeeds, unsigned char *J_set, int *outputLength)
{
	unsigned char *outputBuffer;
	int totalLength = 0, outputOffset = 0, j = 0, k;
	ub4 tempUB4;
	unsigned int temp = 0;


	for(j = 0; j < secret_inputs -> stat_SecParam; j ++)
	{
		if(0x01 == J_set[j])
		{
			totalLength += ( sizeof(mp_limb_t) * mpz_size(secret_inputs -> secret_circuitKeys[j]) );
			totalLength += sizeof(int);
			totalLength += sizeof(unsigned int);
			totalLength += 256 * sizeof(ub4);
		}
	}

	outputBuffer = (unsigned char *) calloc(totalLength, sizeof(unsigned char));

	for(j = 0; j < secret_inputs -> stat_SecParam; j ++)
	{
		if(0x01 == J_set[j])
		{
			serialiseMPZ(secret_inputs -> secret_circuitKeys[j], outputBuffer, &outputOffset);
			memcpy(outputBuffer + outputOffset, seedList + j, sizeof(unsigned int));
			outputOffset += sizeof(unsigned int);

			for(k = 0; k < 256; k ++)
			{
				tempUB4 = circuitSeeds[j][k];
				memcpy(outputBuffer + outputOffset, &tempUB4 + j, sizeof(ub4));
				//outputOffset +=  256 * sizeof(ub4);
				outputOffset +=  sizeof(ub4);
			}
		}
	}

	*outputLength = outputOffset;

	return outputBuffer;
}


struct revealedCheckSecrets *deserialise_Requested_CircuitSecrets(unsigned char *inputBuffer, int stat_SecParam,
											unsigned char *J_set,
											struct public_builderPRS_Keys *public_input,
											struct eccParams *params)
{
	int inputOffset = 0, j = 0, k;

	struct revealedCheckSecrets *outputStruct = (struct revealedCheckSecrets *) calloc(1, sizeof(struct revealedCheckSecrets));
	outputStruct -> revealedSecrets = (mpz_t*) calloc(stat_SecParam, sizeof(mpz_t));
	unsigned int *revealedSeeds = (unsigned int*) calloc(stat_SecParam, sizeof(unsigned int));
	ub4 **revealedCircuitSeeds = (ub4 **) calloc(stat_SecParam, sizeof(ub4 *));
	mpz_t *tempMPZ, scratchMPZ;
	struct eccPoint *scratchPoint;
	ub4 tempUB4;

	mpz_init(scratchMPZ);


	for(j = 0; j < stat_SecParam; j ++)
	{

		if(0x01 == J_set[j])
		{
			tempMPZ = deserialiseMPZ(inputBuffer, &inputOffset);

			mpz_init(outputStruct -> revealedSecrets[j]);
			mpz_set(outputStruct -> revealedSecrets[j], *tempMPZ);

			scratchPoint = windowedScalarPoint(outputStruct -> revealedSecrets[j], params -> g, params);


			if(0 != eccPointsEqual(scratchPoint, public_input -> public_circuitKeys[j]))
			{
				printf("Drat\n");
				fflush(stdout);
				return NULL;
			}
			memcpy(revealedSeeds + j, inputBuffer + inputOffset, sizeof(unsigned int));
			inputOffset += sizeof(unsigned int);

			revealedCircuitSeeds[j] = (ub4 *) calloc(256, sizeof(ub4));
			for(k = 0; k < 256; k ++)
			{
				memcpy(&tempUB4, inputBuffer + inputOffset, sizeof(ub4));
				revealedCircuitSeeds[j][k] = tempUB4;
				inputOffset += sizeof(ub4);
			}
			// free(tempMPZ);
		}
	}

	outputStruct -> revealedSeeds = revealedSeeds;
	outputStruct -> revealedCircuitSeeds = revealedCircuitSeeds;

	return outputStruct;
}


