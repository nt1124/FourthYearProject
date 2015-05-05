// Init the structure containing the secret keys for the inputs
struct secret_builderPRS_Keys *init_secret_input_keys(int numInputs, int stat_SecParam)
{
	struct secret_builderPRS_Keys *toReturn = (struct secret_builderPRS_Keys*) calloc(1, sizeof(struct secret_builderPRS_Keys));
	int i;

	toReturn -> numKeyPairs = numInputs;
	toReturn -> secret_keyPairs = (mpz_t **) calloc(numInputs, sizeof(mpz_t*));

	// For each input wire
	for(i = 0; i < numInputs; i ++)
	{
		toReturn -> secret_keyPairs[i] = (mpz_t *) calloc(2, sizeof(mpz_t));

		mpz_init(toReturn -> secret_keyPairs[i][0]);
		mpz_init(toReturn -> secret_keyPairs[i][1]);
	}

	toReturn -> stat_SecParam = stat_SecParam;
	toReturn -> secret_circuitKeys = (mpz_t *) calloc(stat_SecParam, sizeof(mpz_t));

	// For each circuit we'll be building.
	for(i = 0; i < stat_SecParam; i ++)
	{
		mpz_init(toReturn -> secret_circuitKeys[i]);
	}

	return toReturn;
}


// Initialise the structure holding the secrets revealed for the J-set
struct secret_builderPRS_Keys *init_check_comp_secrets(int numInputs, int stat_SecParam, struct secret_builderPRS_Keys *mainSecrets)
{
	struct secret_builderPRS_Keys *toReturn = (struct secret_builderPRS_Keys*) calloc(1, sizeof(struct secret_builderPRS_Keys));
	int i;

	toReturn -> numKeyPairs = numInputs;
	toReturn -> secret_keyPairs = (mpz_t **) calloc(numInputs, sizeof(mpz_t*));

	// For each input wire
	for(i = 0; i < numInputs; i ++)
	{
		toReturn -> secret_keyPairs[i] = (mpz_t *) calloc(2, sizeof(mpz_t));

		mpz_init_set(toReturn -> secret_keyPairs[i][0], mainSecrets -> secret_keyPairs[i][0]);
		mpz_init_set(toReturn -> secret_keyPairs[i][1], mainSecrets -> secret_keyPairs[i][1]);
	}

	toReturn -> stat_SecParam = stat_SecParam;
	toReturn -> secret_circuitKeys = (mpz_t *) calloc(stat_SecParam, sizeof(mpz_t));

	// We init even those we don't need just for ease.
	for(i = 0; i < stat_SecParam; i ++)
	{
		mpz_init(toReturn -> secret_circuitKeys[i]);
	}

	return toReturn;
}


// Initialise the public key structure.
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


// House keeping
void free_secret_key_set(struct secret_builderPRS_Keys *toFree)
{
	int i;

	for(i = 0; i < toFree -> numKeyPairs; i ++)
	{
		free(toFree -> secret_keyPairs[i]);
	}

	free(toFree -> secret_circuitKeys);
}

// House keeping
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


// Generate a set of secrets, both input and circuits.
struct secret_builderPRS_Keys *generateSecrets(int numInputs, int stat_SecParam, struct eccParams *params, gmp_randstate_t state)
{
	struct secret_builderPRS_Keys *secret_inputs;
	int i;
	

	secret_inputs = init_secret_input_keys(numInputs, stat_SecParam);

	// For each input wire, both 0-value and 1-value.
	for(i = 0; i < secret_inputs -> numKeyPairs; i ++)
	{
		mpz_urandomm(secret_inputs -> secret_keyPairs[i][0], state, params -> n);
		mpz_urandomm(secret_inputs -> secret_keyPairs[i][1], state, params -> n);
	}

	// For each circuit.
	for(i = 0; i < secret_inputs -> stat_SecParam; i ++)
	{
		mpz_urandomm(secret_inputs -> secret_circuitKeys[i], state, params -> n);
	}


	return secret_inputs;
}


// Generate secrets for ONLY the circuits. For use in Lindell 2013 protocol when we need new circuit
// secrets for the sub-computation circuits, but will be using the same input key secrets.
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


// Use the secrets to compute the public commitments for the secrets.
// Note the parallel doesn't help much directly but seems to help the memory allocation.
struct public_builderPRS_Keys *computePublicInputs(struct secret_builderPRS_Keys *secret_inputs, struct eccParams *params)
{
	struct public_builderPRS_Keys *public_inputs;
	int i;


	public_inputs = init_public_input_keys(secret_inputs -> numKeyPairs, secret_inputs -> stat_SecParam);

	#pragma omp parallel for default(shared) private(i) schedule(auto)
	for(i = 0; i < secret_inputs -> numKeyPairs; i ++)
	{
		public_inputs -> public_keyPairs[i][0] = fixedPointMultiplication(gPreComputes, secret_inputs -> secret_keyPairs[i][0], params);
		public_inputs -> public_keyPairs[i][1] = fixedPointMultiplication(gPreComputes, secret_inputs -> secret_keyPairs[i][1], params);
	}

	#pragma omp parallel for default(shared) private(i) schedule(auto)
	for(i = 0; i < secret_inputs -> stat_SecParam; i ++)
	{
		public_inputs -> public_circuitKeys[i] = fixedPointMultiplication(gPreComputes, secret_inputs -> secret_circuitKeys[i], params);
	}

	return public_inputs;
}


// Serialise the public commitments for the secrets. Returns a uchar buffer with the serialised commits.
unsigned char *serialisePublicInputs(struct public_builderPRS_Keys *public_inputs, int *outputLength)
{
	unsigned char *outputBuffer;
	int totalLength = 0, i, tempNumKeyPairs, tempStat_SecParam;
	int outputOffset = sizeof(int);


	// First find out how big the buffer needs to be.
	for(i = 0; i < public_inputs -> numKeyPairs; i ++)
	{
		totalLength += sizeOfSerial_ECCPoint(public_inputs -> public_keyPairs[i][0]);
		totalLength += sizeOfSerial_ECCPoint(public_inputs -> public_keyPairs[i][1]);
	}
	for(i = 0; i < public_inputs -> stat_SecParam; i ++)
	{
		totalLength += sizeOfSerial_ECCPoint(public_inputs -> public_circuitKeys[i]);
	}

	// Add length for some metadata.
	totalLength += (2 * sizeof(int));

	tempNumKeyPairs = public_inputs -> numKeyPairs;
	tempStat_SecParam = public_inputs -> stat_SecParam;

	// Start the buffer with the number of each type of item to expect.
	outputBuffer = (unsigned char*) calloc(totalLength, sizeof(unsigned char));
	memcpy(outputBuffer, &tempNumKeyPairs, sizeof(int));
	memcpy(outputBuffer + outputOffset, &tempStat_SecParam, sizeof(int));
	outputOffset += sizeof(int);

	// Then serialise each input wire commit, 0-value then 1-value then next input wire.
	for(i = 0; i < public_inputs -> numKeyPairs; i ++)
	{
		serialise_ECC_Point(public_inputs -> public_keyPairs[i][0], outputBuffer, &outputOffset);
		serialise_ECC_Point(public_inputs -> public_keyPairs[i][1], outputBuffer, &outputOffset);
	}

	// Then each circuit secret commit.
	for(i = 0; i < public_inputs -> stat_SecParam; i ++)
	{
		serialise_ECC_Point(public_inputs -> public_circuitKeys[i], outputBuffer, &outputOffset);
	}

	// Set the output length so the outside function knows how big the buffer is for sending purposes.
	*outputLength = outputOffset;

	return outputBuffer;
}


// Function to take a uchar buffer and deserialise it into a set of public commits to the secret sets.
struct public_builderPRS_Keys *deserialisePublicInputs(unsigned char *inputBuffer, int *bufferOffset)
{
	struct public_builderPRS_Keys *toReturn;
	int i, inputOffset = *bufferOffset, numKeyPairs, stat_SecParam;
	mpz_t *tempMPZ;


	// Read the metadata.
	memcpy(&numKeyPairs, inputBuffer + inputOffset, sizeof(int));
	inputOffset += sizeof(int);
	memcpy(&stat_SecParam, inputBuffer + inputOffset, sizeof(int));
	inputOffset += sizeof(int);

	// Init output structure.
	toReturn = init_public_input_keys(numKeyPairs, stat_SecParam);


	// For each input wire,
	for(i = 0; i < toReturn -> numKeyPairs; i ++)
	{
		toReturn -> public_keyPairs[i][0] = deserialise_ECC_Point(inputBuffer, &inputOffset);
		toReturn -> public_keyPairs[i][1] = deserialise_ECC_Point(inputBuffer, &inputOffset);
	}

	// For each circuit,
	for(i = 0; i < toReturn -> stat_SecParam; i ++)
	{
		toReturn -> public_circuitKeys[i] = deserialise_ECC_Point(inputBuffer, &inputOffset);
	}

	// Update the buffer offset, means this can be read from the middle of a bigger buffer.
	*bufferOffset = inputOffset;

	return toReturn;
}


// Take a J-set and serialise the secrets for all circuits indicated by this J-set.
// Also serialise the seeds of the randomness used to build each circuit.
unsigned char *serialise_Requested_CircuitSecrets(struct secret_builderPRS_Keys *secret_inputs,
												ub4 **circuitSeeds, unsigned char *J_set, int *outputLength)
{
	unsigned char *outputBuffer;
	int totalLength = 0, outputOffset = 0, j = 0, k;
	ub4 tempUB4;
	unsigned int temp = 0;


	// Calculate the size of the buffer needed.
	for(j = 0; j < secret_inputs -> stat_SecParam; j ++)
	{
		if(0x01 == J_set[j])
		{
			// Int for size of MPZ.
			totalLength += sizeof(int);
			totalLength += ( sizeof(mp_limb_t) * mpz_size(secret_inputs -> secret_circuitKeys[j]) );
			// Size of seed.
			totalLength += 256 * sizeof(ub4);
		}
	}

	outputBuffer = (unsigned char *) calloc(totalLength, sizeof(unsigned char));

	// For each circuit in the J-set.
	for(j = 0; j < secret_inputs -> stat_SecParam; j ++)
	{
		if(0x01 == J_set[j])
		{
			// Serialise circuit secret
			serialiseMPZ(secret_inputs -> secret_circuitKeys[j], outputBuffer, &outputOffset);

			// Serialise circuti seed.
			for(k = 0; k < 256; k ++)
			{
				tempUB4 = circuitSeeds[j][k];
				memcpy(outputBuffer + outputOffset, &tempUB4, sizeof(ub4));
				outputOffset += sizeof(ub4);
			}
		}
	}

	// Set output length for sending purposes.
	*outputLength = outputOffset;

	return outputBuffer;
}


// Deserialise the revealed circuit secrets for the purpose of checking circuits.
struct revealedCheckSecrets *deserialise_Requested_CircuitSecrets(unsigned char *inputBuffer, int stat_SecParam,
											unsigned char *J_set,
											struct public_builderPRS_Keys *public_input,
											struct eccParams *params)
{
	int inputOffset = 0, j = 0, k;

	struct revealedCheckSecrets *outputStruct = (struct revealedCheckSecrets *) calloc(1, sizeof(struct revealedCheckSecrets));
	outputStruct -> revealedSecrets = (mpz_t*) calloc(stat_SecParam, sizeof(mpz_t));
	ub4 **revealedCircuitSeeds = (ub4 **) calloc(stat_SecParam, sizeof(ub4 *));
	mpz_t *tempMPZ, scratchMPZ;
	struct eccPoint *scratchPoint;
	ub4 tempUB4;


	mpz_init(scratchMPZ);

	// For each circuit in the J-set
	for(j = 0; j < stat_SecParam; j ++)
	{
		if(0x01 == J_set[j])
		{
			// Deserialise the circuit secret
			tempMPZ = deserialiseMPZ(inputBuffer, &inputOffset);

			// Check the deserialised secret agrees with the relevant commitment.
			mpz_init(outputStruct -> revealedSecrets[j]);
			mpz_set(outputStruct -> revealedSecrets[j], *tempMPZ);

			scratchPoint = windowedScalarPoint(outputStruct -> revealedSecrets[j], params -> g, params);

			if(0 != eccPointsEqual(scratchPoint, public_input -> public_circuitKeys[j]))
			{
				printf("Drat\n");
				fflush(stdout);
				return NULL;
			}

			// Deserialise the circuit seed.
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


	outputStruct -> revealedCircuitSeeds = revealedCircuitSeeds;

	return outputStruct;
}


// Use the circuit secrets to compute the POINTs (not hashes) representing the input key pairs
// to each circuit.
struct eccPoint ***getAllConsistentInputsAsPoints(mpz_t *circuitSecrets, struct eccPoint ***publicKeyPairs, struct eccParams *params,
												int numCircuits, int numInputs)
{
	struct eccPoint ***outputPoints = (struct eccPoint ***) calloc(numCircuits, sizeof(struct eccPoint **));
	int i, j, k;


	// For each circuit
	#pragma omp parallel for default(shared) private(i, j, k) schedule(auto)
	for(i = 0; i < numCircuits; i ++)
	{
		outputPoints[i] = (struct eccPoint **) calloc(2 * numInputs, sizeof(struct eccPoint *));

		// For each input wire
		for(j = 0; j < numInputs; j ++)
		{
			k = 2 * j;

			// Raise the relevant public input wire value to the power of the circuit secret.
			outputPoints[i][k + 0] = windowedScalarPoint(circuitSecrets[i], publicKeyPairs[j][0], params);
			outputPoints[i][k + 1] = windowedScalarPoint(circuitSecrets[i], publicKeyPairs[j][1], params);
		}
	}


	return outputPoints;
}


// Use the circuit secrets to compute the POINTs (not hashes) representing the input key pairs
// to each circuit in the J-set.
struct eccPoint ***getAllConsistentInputsAsPoints(mpz_t *circuitSecrets, struct eccPoint ***publicKeyPairs, struct eccParams *params,
												unsigned char *J_set, int numCircuits, int numInputs)
{
	struct eccPoint ***outputPoints = (struct eccPoint ***) calloc(numCircuits, sizeof(struct eccPoint **));
	int i, j, k;


	// For each circuit in the J-set
	#pragma omp parallel for default(shared) private(i, j, k) schedule(auto)
	for(i = 0; i < numCircuits; i ++)
	{
		if(0x01 == J_set[i])
		{
			outputPoints[i] = (struct eccPoint **) calloc(2 * numInputs, sizeof(struct eccPoint *));

			// For each input wire
			for(j = 0; j < numInputs; j ++)
			{
				k = 2 * j;

				// Raise the relevant public input wire value to the power of the circuit secret.
				outputPoints[i][k + 0] = windowedScalarPoint(circuitSecrets[i], publicKeyPairs[j][0], params);
				outputPoints[i][k + 1] = windowedScalarPoint(circuitSecrets[i], publicKeyPairs[j][1], params);
			}
		}
	}

	return outputPoints;
}


