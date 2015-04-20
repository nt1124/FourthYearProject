struct builderInputCommitStruct *initBuilderInputCommitStruct(int iMax, int jMax, gmp_randstate_t *state)
{
	struct builderInputCommitStruct *outputStruct = (struct builderInputCommitStruct *) calloc(1, sizeof(struct builderInputCommitStruct));
	int i, j;


	outputStruct -> params = generate_commit_params(1024, *state);
	outputStruct -> commitmentPerms = (unsigned char **) calloc(iMax, sizeof(unsigned char *));
	outputStruct -> c_boxes_0 = (struct elgamal_commit_box ***) calloc(iMax, sizeof(struct elgamal_commit_box **));
	outputStruct -> c_boxes_1 = (struct elgamal_commit_box ***) calloc(iMax, sizeof(struct elgamal_commit_box **));
	outputStruct -> k_boxes_0 = (struct elgamal_commit_key ***) calloc(iMax, sizeof(struct elgamal_commit_key **));
	outputStruct -> k_boxes_1 = (struct elgamal_commit_key ***) calloc(iMax, sizeof(struct elgamal_commit_key **));

	outputStruct -> iMax = iMax;
	outputStruct -> jMax = jMax;


	for(i = 0; i < iMax; i ++)
	{
		outputStruct -> commitmentPerms[i] = (unsigned char *) calloc(jMax, sizeof(unsigned char));
		outputStruct -> c_boxes_0[i] = (struct elgamal_commit_box **) calloc(jMax, sizeof(struct elgamal_commit_box *));
		outputStruct -> c_boxes_1[i] = (struct elgamal_commit_box **) calloc(jMax, sizeof(struct elgamal_commit_box *));
		outputStruct -> k_boxes_0[i] = (struct elgamal_commit_key **) calloc(jMax, sizeof(struct elgamal_commit_key *));
		outputStruct -> k_boxes_1[i] = (struct elgamal_commit_key **) calloc(jMax, sizeof(struct elgamal_commit_key *));

		for(j = 0; j < jMax; j ++)
		{
			outputStruct -> c_boxes_0[i][j] = init_commit_box();
			outputStruct -> c_boxes_1[i][j] = init_commit_box();

			outputStruct -> k_boxes_0[i][j] = init_commit_key();
			outputStruct -> k_boxes_1[i][j] = init_commit_key();
		}
	}


	return outputStruct;
}


// This function computes all the commitments a party needs to make for its inputs.
struct builderInputCommitStruct *makeCommitmentsBuilder(randctx *ctx, struct Circuit **circuitsArray, gmp_randstate_t *state, int numCircuits)
{
	struct builderInputCommitStruct *outputStruct = initBuilderInputCommitStruct(numCircuits, circuitsArray[0] -> numInputsBuilder, state);
	struct wire *tempWire;
	int i, j, k, gateIndex;
	unsigned char *temp0, *temp1;


	// iMax = numCircuits
	for(i = 0; i < outputStruct -> iMax; i ++)
	{
		// jMax = numInputs
		for(j = 0; j < outputStruct -> jMax; j ++)
		{
			// We permute the commitment pair so that when decommiting to the eval circuit inputs
			// the other party doesn't know which bit our input represents.
			outputStruct -> commitmentPerms[i][j] = (0x01 & getIsaacPermutation(ctx));
		}
	}

	// iMax = numCircuits
	#pragma omp parallel for default(shared) private(i, j, temp0, temp1, gateIndex, tempWire) 
	for(i = 0; i < outputStruct -> iMax; i ++)
	{
		// jMax = numInputs
		for(j = 0; j < outputStruct -> jMax; j ++)
		{
			gateIndex = circuitsArray[i] -> builderInputOffset + j;
			tempWire = circuitsArray[i] -> gates[gateIndex] -> outputWire;

			if(0 == outputStruct -> commitmentPerms[i][j])
			{
				temp0 = tempWire -> outputGarbleKeys -> key0;
				temp1 = tempWire -> outputGarbleKeys -> key1;
			}
			else
			{
				temp0 = tempWire -> outputGarbleKeys -> key1;
				temp1 = tempWire -> outputGarbleKeys -> key0;
			}

			create_commit_box_key(outputStruct -> params, temp0, 16, *state, outputStruct -> c_boxes_0[i][j], outputStruct -> k_boxes_0[i][j]);
			create_commit_box_key(outputStruct -> params, temp1, 16, *state, outputStruct -> c_boxes_1[i][j], outputStruct -> k_boxes_1[i][j]);
		}
	}


	return outputStruct;
}



unsigned char *serialiseC_Boxes(struct builderInputCommitStruct *commitStruct, int *totalLength)
{
	unsigned char *outputBuffer, *temp0, *temp1, *temp2;
	int outputLength = 0, tempOffset0, tempOffset1, tempOffset2, temp2Length;


	tempOffset0 = 0;
	temp0 = serialiseAllC_Boxes_2D(commitStruct -> c_boxes_0, commitStruct -> iMax, commitStruct -> jMax, &tempOffset0);
	outputLength += tempOffset0;

	tempOffset1 = 0;
	temp1 = serialiseAllC_Boxes_2D(commitStruct -> c_boxes_1, commitStruct -> iMax, commitStruct -> jMax, &tempOffset1);
	outputLength += tempOffset1;

	tempOffset2 = 0;
	temp2 = serialise_commit_batch_params(commitStruct -> params, &tempOffset2);
	outputLength += tempOffset2;


	outputBuffer = (unsigned char *) calloc(outputLength, sizeof(unsigned char));

	memcpy(outputBuffer, temp0, tempOffset0);
	memcpy(outputBuffer + tempOffset0, temp1, tempOffset1);
	memcpy(outputBuffer + tempOffset0 + tempOffset1, temp2, tempOffset2);
	free(temp0);
	free(temp1);
	free(temp2);


	*totalLength = tempOffset0 + tempOffset1 + tempOffset2;

	return outputBuffer;
}


struct builderInputCommitStruct *deserialiseC_Boxes(unsigned char *inputBuffer, int iMax, int jMax, gmp_randstate_t *state, int *inputOffset)
{
	struct builderInputCommitStruct *outputStruct = (struct builderInputCommitStruct *) calloc(1, sizeof(struct builderInputCommitStruct));
	int i, j, tempOffset = *inputOffset;


	outputStruct -> commitmentPerms = (unsigned char **) calloc(iMax, sizeof(unsigned char *));
	outputStruct -> c_boxes_0 = (struct elgamal_commit_box ***) calloc(iMax, sizeof(struct elgamal_commit_box **));
	outputStruct -> c_boxes_1 = (struct elgamal_commit_box ***) calloc(iMax, sizeof(struct elgamal_commit_box **));

	outputStruct -> iMax = iMax;
	outputStruct -> jMax = jMax;


	for(i = 0; i < iMax; i ++)
	{
		outputStruct -> commitmentPerms[i] = (unsigned char *) calloc(jMax, sizeof(unsigned char));
	}

	outputStruct -> c_boxes_0 = deserialiseAllC_Boxes_2D(inputBuffer, iMax, jMax, &tempOffset);
	outputStruct -> c_boxes_1 = deserialiseAllC_Boxes_2D(inputBuffer, iMax, jMax, &tempOffset);


	outputStruct -> params = deserialise_commit_batch_params(inputBuffer, &tempOffset);


	return outputStruct;
}



// Note that the Receiver already has the x from the key boxes.
unsigned char *decommitToCheckCircuitInputs_Builder(struct builderInputCommitStruct *commitStruct, unsigned char *inputBitsOwn, unsigned char *jSetPartner, int *outputLength)
{
	unsigned char *outputBuffer, permedBit;
	int i, j, bufferOffset = 0;
	int tempBytesCount = commitStruct -> jMax * sizeof(unsigned char);

	int validCommitments, h;


	*outputLength = commitStruct -> iMax * tempBytesCount;

	for(i = 0; i < commitStruct -> iMax; i ++)
	{
		if(0x01 == jSetPartner[i])
		{
			for(j = 0; j < commitStruct -> jMax; j ++)
			{
				(*outputLength) += sizeOfSerialMPZ(commitStruct -> k_boxes_0[i][j] -> r);
				(*outputLength) += sizeOfSerialMPZ(commitStruct -> k_boxes_1[i][j] -> r);
			}
		}
		else
		{
			for(j = 0; j < commitStruct -> jMax; j ++)
			{
				if(0x00 == inputBitsOwn[j])
				{
					(*outputLength) += (1 + sizeOfSerialMPZ(commitStruct -> k_boxes_0[i][j] -> r));
				}
				else
				{
					(*outputLength) += (1 + sizeOfSerialMPZ(commitStruct -> k_boxes_1[i][j] -> r));
				}
			}
		}
	}

	outputBuffer = (unsigned char *) calloc(*outputLength, sizeof(unsigned char));

	for(i = 0; i < commitStruct -> iMax; i ++)
	{
		if(0x01 == jSetPartner[i])
		{
			memcpy(outputBuffer + bufferOffset, commitStruct -> commitmentPerms[i], tempBytesCount);
			bufferOffset += tempBytesCount;

			for(j = 0; j < commitStruct -> jMax; j ++)
			{
				serialiseMPZ(commitStruct -> k_boxes_0[i][j] -> r, outputBuffer, &bufferOffset);
				serialiseMPZ(commitStruct -> k_boxes_1[i][j] -> r, outputBuffer, &bufferOffset);
			}
		}
	}

	for(i = 0; i < commitStruct -> iMax; i ++)
	{
		if(0x00 == jSetPartner[i])
		{
			for(j = 0; j < commitStruct -> jMax; j ++)
			{
				outputBuffer[bufferOffset] = permedBit = (inputBitsOwn[j] ^ commitStruct -> commitmentPerms[i][j]);
				bufferOffset ++;

				if(0x00 == permedBit)
				{
					serialiseMPZ(commitStruct -> k_boxes_0[i][j] -> r, outputBuffer, &bufferOffset);
				}
				else
				{
					serialiseMPZ(commitStruct -> k_boxes_1[i][j] -> r, outputBuffer, &bufferOffset);
				}
			}
		}
	}

	return outputBuffer;
}


// Note that the Receiver already has the x from the key boxes.
int decommitToCheckCircuitInputs_Exec(struct builderInputCommitStruct *commitStruct, struct jSetRevealHKE *partnerReveals, struct eccPoint ***NaorPinkasInputs_Partner,
									unsigned char *inputBuffer, unsigned char *jSetOwn, int numCircuits, int numInputs, int *inputOffset)
{
	unsigned char permedBit, **permutations = (unsigned char **) calloc(numCircuits, sizeof(unsigned char *)), *x0, *x1;
	int i, j, k, tempOffset = *inputOffset, validCommitments = 0;
	int tempBytesCount = numInputs * sizeof(unsigned char);
	mpz_t *tempR0, *tempR1;


	for(i = 0; i < numCircuits; i ++)
	{
		if(0x01 == jSetOwn[i])
		{
			k = 0;
			permutations[i] = (unsigned char *) calloc(numInputs, sizeof(unsigned char));
			memcpy(permutations[i], inputBuffer + tempOffset, tempBytesCount);
			tempOffset += tempBytesCount;

			for(j = 0; j < numInputs; j ++)
			{
				tempR0 = deserialiseMPZ(inputBuffer, &tempOffset);
				x0 = hashECC_Point(NaorPinkasInputs_Partner[i][k], 16);
				k ++;

				tempR1 = deserialiseMPZ(inputBuffer, &tempOffset);
				x1 = hashECC_Point(NaorPinkasInputs_Partner[i][k], 16);
				k ++;

				if(0x00 == permutations[i][j])
				{
					validCommitments |= single_decommit_raw_elgamal_R(commitStruct -> params, commitStruct -> c_boxes_0[i][j], x0, *tempR0, 16);
					validCommitments |= single_decommit_raw_elgamal_R(commitStruct -> params, commitStruct -> c_boxes_1[i][j], x1, *tempR1, 16);
				}
				else
				{
					validCommitments |= single_decommit_raw_elgamal_R(commitStruct -> params, commitStruct -> c_boxes_0[i][j], x1, *tempR0, 16);
					validCommitments |= single_decommit_raw_elgamal_R(commitStruct -> params, commitStruct -> c_boxes_1[i][j], x0, *tempR1, 16);
				}

				mpz_clear(*tempR0);
				free(tempR0);
				free(x0);

				mpz_clear(*tempR1);
				free(tempR1);
				free(x1);
			}
		}
	}


	// Now check that the 
	for(i = 0; i < numCircuits; i ++)
	{
		if(0x00 == jSetOwn[i])
		{
			for(j = 0; j < numInputs; j ++)
			{
				permedBit = inputBuffer[tempOffset ++];

				x0 = hashECC_Point(partnerReveals -> builderInputsEval[i][j], 16);
				tempR0 = deserialiseMPZ(inputBuffer, &tempOffset);

				if(0x00 == permedBit)
				{
					validCommitments |= single_decommit_raw_elgamal_R(commitStruct -> params, commitStruct -> c_boxes_0[i][j], x0, *tempR0, 16);
				}
				else
				{
					validCommitments |= single_decommit_raw_elgamal_R(commitStruct -> params, commitStruct -> c_boxes_1[i][j], x0, *tempR0, 16);
				}

				mpz_clear(*tempR0);
				free(tempR0);
			}
		}
	}


	*inputOffset = tempOffset;


	return validCommitments;
}