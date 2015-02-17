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
										int i, int j, unsigned char inputBit)
{
	mpz_t mpz_representation;
	unsigned char *rawBytes, *hashedBytes, *halfHash = (unsigned char *) calloc(16, sizeof(unsigned char));
	int outputLength = 0;

	mpz_init(mpz_representation);


	mpz_powm(mpz_representation, public_inputs -> public_keyPairs[i][inputBit], secret_inputs -> secret_circuitKeys[j], group -> p);

	rawBytes = convertMPZToBytes(mpz_representation, &outputLength);

	// hashedBytes = sha_256_hash(rawBytes, outputLength);

	memcpy(halfHash, rawBytes, 16);

	free(rawBytes);

	return halfHash;
}
