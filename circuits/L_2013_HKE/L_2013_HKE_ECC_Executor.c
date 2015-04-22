
struct jSetReveal_L_2013_HKE *jSetRevealDeserialise_L_2013_HKE(unsigned char *inputBuffer, unsigned char *jSet, int numInputs, int numCircuits)
{
	struct jSetReveal_L_2013_HKE *output = (struct jSetReveal_L_2013_HKE *) calloc(1, sizeof(struct jSetReveal_L_2013_HKE));
	mpz_t *tempMPZ;
	int i, j, k, tempOffset = 0;


	output -> aListRevealed = (mpz_t **) calloc(numCircuits, sizeof(mpz_t *));
	output -> revealedSeeds = (ub4 **) calloc(numCircuits, sizeof(ub4*));
	output -> builderInputsEval = (struct eccPoint ***) calloc(numCircuits, sizeof(struct eccPoint **));


	for(j = 0; j < numCircuits; j ++)
	{
		if(0x01 == jSet[j])
		{
			k = 0;
			output -> aListRevealed[j] = (mpz_t *) calloc(2 * numInputs, sizeof(mpz_t));
			output -> revealedSeeds[j] = (ub4 *) calloc(256, sizeof(ub4));

			for(i = 0; i < numInputs; i ++)
			{
				tempMPZ = deserialiseMPZ(inputBuffer, &tempOffset);
				mpz_init_set(output -> aListRevealed[j][k], *tempMPZ);
				k ++;

				tempMPZ = deserialiseMPZ(inputBuffer, &tempOffset);
				mpz_init_set(output -> aListRevealed[j][k], *tempMPZ);
				k ++;
			}

			memcpy(output -> revealedSeeds[j], inputBuffer + tempOffset, 256 * sizeof(ub4));
			tempOffset += 256 * sizeof(ub4);
		}
	}

	for(j = 0; j < numCircuits; j ++)
	{
		if(0x00 == jSet[j])
		{
			output -> builderInputsEval[j] = (struct eccPoint **) calloc(numInputs, sizeof(struct eccPoint *));
			for(i = 0; i < numInputs; i ++)
			{
				output -> builderInputsEval[j][i] = deserialise_ECC_Point(inputBuffer, &tempOffset);
			}
		}
	}


	return output;
}



struct jSetReveal_L_2013_HKE *executor_ToJ_Set_L_2013_HKE(int writeSocket, int readSocket, struct Circuit **circuitsArray,
														struct eccParams *params, unsigned char *J_set, int *J_setSize, int stat_SecParam)
{
	struct jSetReveal_L_2013_HKE *revealedSecrets;
	struct wire *tempWire;
	unsigned char *commBuffer;
	int i, commBufferLen, tempOffset = stat_SecParam * sizeof(unsigned char);
	int count = 0;


	for(i = 0; i < stat_SecParam; i ++)
	{
		count += J_set[i];		
	}
	*J_setSize = count;
	
	// We want to send the whole J_set and then to send both keys for a wire in each circuit in the J_Set.
	commBufferLen = stat_SecParam + (16 * 2) * count;
	commBuffer = (unsigned char*) calloc(commBufferLen, sizeof(unsigned char));

	memcpy(commBuffer, J_set, stat_SecParam * sizeof(unsigned char));


	// For each circuit we send both value on our first input wire wire, thus proving that
	// we indeed opened that circuit for checking (i.e. It's J_set value was 1).
	for(i = 0; i < stat_SecParam; i ++)
	{
		if(0x01 == J_set[i])
		{
			tempWire = circuitsArray[i] -> gates[circuitsArray[i] -> numInputsBuilder] -> outputWire;

			memcpy(commBuffer + tempOffset, tempWire -> outputGarbleKeys -> key0, 16);
			tempOffset += 16;

			memcpy(commBuffer + tempOffset, tempWire -> outputGarbleKeys -> key1, 16);
			tempOffset += 16;
		}
	}

	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);


	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);


	if(1 != commBufferLen)
	{
		revealedSecrets = jSetRevealDeserialise_L_2013_HKE(commBuffer, J_set, circuitsArray[0] -> numInputsBuilder, stat_SecParam);
		free(commBuffer);

		return revealedSecrets;
	}

	printf("Drat!\n");


	return NULL;
}
