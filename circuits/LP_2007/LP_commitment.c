struct commit_pair_Keys *LP_2007_commit_oneP2_C(struct commit_batch_params *params, gmp_randstate_t state,
												struct Circuit **circuitsArray, int circuitIndex, int wireIndex,
												unsigned char *bufferToSend, int *bufferOffset)
{
	struct commit_pair_Keys *keysToReturn = (struct commit_pair_Keys*) calloc(1, sizeof(struct commit_pair_Keys));
	struct wire *tempWire = circuitsArray[circuitIndex] -> gates[wireIndex] -> outputWire;
	int tempOffset = *bufferOffset;

	// They only need the key, not the permutation as they have the permutation.
	keysToReturn -> Key_0 = commit_hash_elgamal_C(params, tempWire -> outputGarbleKeys -> key0, 16, bufferToSend, &tempOffset, state);
	keysToReturn -> Key_1 = commit_hash_elgamal_C(params, tempWire -> outputGarbleKeys -> key1, 16, bufferToSend, &tempOffset, state);

	*bufferOffset = tempOffset;

	return keysToReturn;
}

struct commit_pair_Boxes *LP_2007_commit_oneP2_R(struct commit_batch_params *params,
												unsigned char *bufferReceived, int *bufferOffset)
{
	struct commit_pair_Boxes *boxesToReturn = (struct commit_pair_Boxes*) calloc(1, sizeof(struct commit_pair_Boxes));
	int tempOffset = *bufferOffset;

	// Store them boxes, then return them.
	boxesToReturn -> Box_0 = commit_hash_elgamal_R(params, bufferReceived, &tempOffset);
	boxesToReturn -> Box_1 = commit_hash_elgamal_R(params, bufferReceived, &tempOffset);

	*bufferOffset = tempOffset;

	return boxesToReturn;
}



void LP_2007_decommit_half_oneP2_C(struct commit_batch_params *params, unsigned char bitToDecommit,
								struct commit_pair_Keys *relevantKey,
								unsigned char *bufferToSend, int *bufferOffset)
{
	int tempOffset = *bufferOffset;

	if(0x00 == bitToDecommit)
	{
		decommit_hash_elgamal_C(params, relevantKey -> Key_0, bufferToSend, &tempOffset);
	}
	else
	{
		decommit_hash_elgamal_C(params, relevantKey -> Key_1, bufferToSend, &tempOffset);
	}

	*bufferOffset = tempOffset;
}

unsigned char *LP_2007_decommit_half_oneP2_R(struct commit_batch_params *params,
										struct commit_pair_Boxes *boxToOpen, unsigned char bitToDecommit,
										unsigned char *bufferReceived, int *bufferOffset,
										int *outputLength)
{
	unsigned char *output;
	int tempOffset = *bufferOffset;
	int tempLength;

	// Store them boxes, then return them.
	if(0x00 == bitToDecommit)
	{
		output = decommit_hash_elgamal_R(params, boxToOpen -> Box_0, bufferReceived, &tempOffset, &tempLength);
	}
	else
	{
		output = decommit_hash_elgamal_R(params, boxToOpen -> Box_1, bufferReceived, &tempOffset, &tempLength);
	}

	*bufferOffset = tempOffset;
	*outputLength = tempLength;

	/*
	*outputLength = 2 * sizeof(int) + outputLen_0 + outputLen_1;
	finalOutput = (unsigned char*) calloc( *outputLength, sizeof(unsigned char));

	memcpy(finalOutput, &outputLen_0, sizeof(int));
	tempOffset = sizeof(int);
	memcpy(finalOutput + tempOffset, output_0, outputLen_0);
	tempOffset += outputLen_0;

	memcpy(finalOutput + tempOffset, &outputLen_1, sizeof(int));
	tempOffset += sizeof(int);
	memcpy(finalOutput + tempOffset, output_1, outputLen_1);
	*/

	return output;
}







struct commit_pair_Keys **LP_2007_commit_allP2_C(struct commit_batch_params *params, gmp_randstate_t state,
												struct Circuit **circuitsArray, int secParam_Encoding, int secParam_Circuits,
												unsigned char *bufferToSend, int *bufferOffset)
{
	struct commit_pair_Keys **output;
	int circuitIndex, wireIndex;
	int numInputsP2, numCommits, numInputsBuilder;
	int tempOffset = *bufferOffset, unrolledIndex = 0;

	numInputsBuilder = circuitsArray[0] -> numInputsBuilder;
	numInputsP2 = secParam_Encoding * circuitsArray[0] -> numInputsExecutor;
	numCommits = numInputsP2 * secParam_Circuits;


	output = (struct commit_pair_Keys**) calloc(numCommits, sizeof(struct commit_pair_Keys*));

	for(circuitIndex = 0; circuitIndex < secParam_Circuits; circuitIndex ++)
	{
		for(wireIndex = numInputsBuilder; wireIndex < numInputsBuilder + numInputsP2; wireIndex ++)
		{

			output[unrolledIndex] = LP_2007_commit_oneP2_C(params, state, circuitsArray, circuitIndex, wireIndex,
															bufferToSend, &tempOffset);
			unrolledIndex ++;
		}
	}

	*bufferOffset = tempOffset;

	return output;
}

struct commit_pair_Boxes **LP_2007_commit_allP2_R(struct commit_batch_params *params, int numInputsExecutor,
												int secParam_Encoding, int secParam_Circuits,
												unsigned char *bufferReceived, int *bufferOffset)
{
	struct commit_pair_Boxes **output;
	int circuitIndex, wireIndex;
	int numCommits, numInputsP2, unrolledIndex = 0;
	int tempOffset = *bufferOffset;

	numInputsP2 = secParam_Encoding * numInputsExecutor;
	numCommits = numInputsP2 * secParam_Circuits;


	output = (struct commit_pair_Boxes**) calloc(numCommits, sizeof(struct commit_pair_Boxes*));

	for(circuitIndex = 0; circuitIndex < secParam_Circuits; circuitIndex ++)
	{
		for(wireIndex = 0; wireIndex < numInputsP2; wireIndex ++)
		{

			output[unrolledIndex] = LP_2007_commit_oneP2_R(params, bufferReceived, &tempOffset);
			unrolledIndex ++;
		}
	}

	*bufferOffset = tempOffset;

	return output;
}


void LP_2007_decommit_allP2_C(struct commit_batch_params *params, struct commit_pair_Keys **allKeys,
							struct Circuit **circuitsArray,
							int secParam_Encoding, int secParam_Circuits,
							unsigned char *bufferToSend, int *bufferOffset)
{
	int circuitIndex, wireIndex;
	int numInputsP2, numCommits, numInputsBuilder;
	int tempOffset = *bufferOffset, unrolledIndex = 0;

	numInputsBuilder = circuitsArray[0] -> numInputsBuilder;
	numInputsP2 = secParam_Encoding * circuitsArray[0] -> numInputsExecutor;
	numCommits = numInputsP2 * secParam_Circuits;

	printf(">>>>>>>\n");
	fflush(stdout);

	for(circuitIndex = 0; circuitIndex < secParam_Circuits; circuitIndex ++)
	{
		printf("+ %d\n", circuitIndex);
		fflush(stdout);
		for(wireIndex = 0; wireIndex < numInputsP2; wireIndex ++)
		{
			printf("a %d\n", wireIndex);
			fflush(stdout);
			LP_2007_decommit_half_oneP2_C(params, 0x00, allKeys[unrolledIndex], bufferToSend, &tempOffset);
			
			printf("b %d\n", wireIndex);
			fflush(stdout);
			LP_2007_decommit_half_oneP2_C(params, 0x01, allKeys[unrolledIndex], bufferToSend, &tempOffset);
			unrolledIndex ++;
		}
	}

	*bufferOffset = tempOffset;
}



unsigned char **LP_2007_decommit_allP2_R(struct commit_batch_params *params, struct commit_pair_Boxes **allBoxes,
												int numInputsExecutor, int secParam_Encoding, int secParam_Circuits,
												unsigned char *bufferReceived, int *bufferOffset)
{
	unsigned char **output;
	int circuitIndex, wireIndex;
	int numCommits, numInputsP2, unrolledIndex = 0;
	int tempOffset = *bufferOffset;
	int tempLength = 0;

	numInputsP2 = secParam_Encoding * numInputsExecutor;
	numCommits = numInputsP2 * secParam_Circuits;


	output = (unsigned char**) calloc(2 * numCommits, sizeof(unsigned char*));

	for(circuitIndex = 0; circuitIndex < secParam_Circuits; circuitIndex ++)
	{
		for(wireIndex = 0; wireIndex < numInputsP2; wireIndex ++)
		{
			output[unrolledIndex] = LP_2007_decommit_half_oneP2_R(params, allBoxes[unrolledIndex], 0x00, bufferReceived, &tempOffset, &tempLength);
			output[unrolledIndex + 1] = LP_2007_decommit_half_oneP2_R(params, allBoxes[unrolledIndex], 0x00, bufferReceived, &tempOffset, &tempLength);
			unrolledIndex ++;
		}
	}

	*bufferOffset = tempOffset;

	return output;
}