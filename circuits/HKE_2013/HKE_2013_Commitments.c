struct builderInputCommitStruct *initBuilderInputCommitStruct(int numCircuits, int numInputs, gmp_randstate_t *state)
{
	struct builderInputCommitStruct *outputStruct = (struct builderInputCommitStruct *) calloc(1, sizeof(struct builderInputCommitStruct));
	int i, j;


	outputStruct -> params = generate_commit_params(1024, *state);
	outputStruct -> commitmentPerms = (unsigned char **) calloc(numCircuits, sizeof(unsigned char *));
	outputStruct -> c_boxes_0 = (struct elgamal_commit_box ***) calloc(numCircuits, sizeof(struct elgamal_commit_box **));
	outputStruct -> c_boxes_1 = (struct elgamal_commit_box ***) calloc(numCircuits, sizeof(struct elgamal_commit_box **));
	outputStruct -> k_boxes_0 = (struct elgamal_commit_key ***) calloc(numCircuits, sizeof(struct elgamal_commit_key **));
	outputStruct -> k_boxes_1 = (struct elgamal_commit_key ***) calloc(numCircuits, sizeof(struct elgamal_commit_key **));

	outputStruct -> numCircuits = numCircuits;
	outputStruct -> numInputs = numInputs;


	for(i = 0; i < numCircuits; i ++)
	{
		outputStruct -> commitmentPerms[i] = (unsigned char *) calloc(numInputs, sizeof(unsigned char));
		outputStruct -> c_boxes_0[i] = (struct elgamal_commit_box **) calloc(numInputs, sizeof(struct elgamal_commit_box *));
		outputStruct -> c_boxes_1[i] = (struct elgamal_commit_box **) calloc(numInputs, sizeof(struct elgamal_commit_box *));
		outputStruct -> k_boxes_0[i] = (struct elgamal_commit_key **) calloc(numInputs, sizeof(struct elgamal_commit_key *));
		outputStruct -> k_boxes_1[i] = (struct elgamal_commit_key **) calloc(numInputs, sizeof(struct elgamal_commit_key *));

		for(j = 0; j < numInputs; j ++)
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


	// numCircuits = numCircuits
	for(i = 0; i < outputStruct -> numCircuits; i ++)
	{
		// numInputs = numInputs
		for(j = 0; j < outputStruct -> numInputs; j ++)
		{
			// We permute the commitment pair so that when decommiting to the eval circuit inputs
			// the other party doesn't know which bit our input represents.
			outputStruct -> commitmentPerms[i][j] = (0x01 & getIsaacPermutation(ctx));
		}
	}

	// numCircuits = numCircuits
	#pragma omp parallel for default(shared) private(i, j, temp0, temp1, gateIndex, tempWire) 
	for(i = 0; i < outputStruct -> numCircuits; i ++)
	{
		// numInputs = numInputs
		for(j = 0; j < outputStruct -> numInputs; j ++)
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
	int outputLength = 0, tempOffset, temp0Length, temp1Length, temp2Length;


	temp0Length = 0;
	temp0 = serialiseAllC_Boxes_2D(commitStruct -> c_boxes_0, commitStruct -> numCircuits, commitStruct -> numInputs, &temp0Length);
	outputLength += temp0Length;

	temp1Length = 0;
	temp1 = serialiseAllC_Boxes_2D(commitStruct -> c_boxes_1, commitStruct -> numCircuits, commitStruct -> numInputs, &temp1Length);
	outputLength += temp1Length;

	temp2Length = 0;
	temp2 = serialise_commit_batch_params(commitStruct -> params, &temp2Length);
	outputLength += temp2Length;
	outputLength += 2 * sizeof(int);


	tempOffset = 2 * sizeof(int);
	outputBuffer = (unsigned char *) calloc(outputLength, sizeof(unsigned char));

	memcpy(outputBuffer, &(commitStruct -> numCircuits), sizeof(int));
	memcpy(outputBuffer + sizeof(int), &(commitStruct -> numInputs), sizeof(int));

	memcpy(outputBuffer + tempOffset, temp0, temp0Length);
	tempOffset += temp0Length;

	memcpy(outputBuffer + tempOffset, temp1, temp1Length);
	tempOffset += temp1Length;

	memcpy(outputBuffer + tempOffset, temp2, temp2Length);
	tempOffset += temp2Length;

	free(temp0);
	free(temp1);
	free(temp2);


	*totalLength = tempOffset;

	return outputBuffer;
}


struct builderInputCommitStruct *deserialiseC_Boxes(unsigned char *inputBuffer, gmp_randstate_t *state, int *inputOffset)
{
	struct builderInputCommitStruct *outputStruct = (struct builderInputCommitStruct *) calloc(1, sizeof(struct builderInputCommitStruct));
	int i, j, tempOffset = *inputOffset, tempInt;



	memcpy(&tempInt, inputBuffer + tempOffset, sizeof(int));
	outputStruct -> numCircuits = tempInt;
	tempOffset += sizeof(int);

	memcpy(&tempInt, inputBuffer + tempOffset, sizeof(int));
	outputStruct -> numInputs = tempInt;
	tempOffset += sizeof(int);

	outputStruct -> commitmentPerms = (unsigned char **) calloc(outputStruct -> numCircuits, sizeof(unsigned char *));
	outputStruct -> c_boxes_0 = (struct elgamal_commit_box ***) calloc(outputStruct -> numCircuits, sizeof(struct elgamal_commit_box **));
	outputStruct -> c_boxes_1 = (struct elgamal_commit_box ***) calloc(outputStruct -> numCircuits, sizeof(struct elgamal_commit_box **));


	for(i = 0; i < outputStruct -> numCircuits; i ++)
	{
		outputStruct -> commitmentPerms[i] = (unsigned char *) calloc(outputStruct -> numInputs, sizeof(unsigned char));
	}

	outputStruct -> c_boxes_0 = deserialiseAllC_Boxes_2D(inputBuffer, outputStruct -> numCircuits, outputStruct -> numInputs, &tempOffset);
	outputStruct -> c_boxes_1 = deserialiseAllC_Boxes_2D(inputBuffer, outputStruct -> numCircuits, outputStruct -> numInputs, &tempOffset);


	outputStruct -> params = deserialise_commit_batch_params(inputBuffer, &tempOffset);


	return outputStruct;
}



// Note that the Receiver already has the x from the key boxes.
unsigned char *decommitToCheckCircuitInputs_Builder(struct builderInputCommitStruct *commitStruct, unsigned char *inputBitsOwn, unsigned char *jSetPartner, int *outputLength)
{
	unsigned char *outputBuffer, permedBit;
	int i, j, bufferOffset = 0;
	int tempBytesCount = commitStruct -> numInputs * sizeof(unsigned char);

	int validCommitments, h;


	*outputLength = commitStruct -> numCircuits * tempBytesCount;

	for(i = 0; i < commitStruct -> numCircuits; i ++)
	{
		if(0x01 == jSetPartner[i])
		{
			for(j = 0; j < commitStruct -> numInputs; j ++)
			{
				(*outputLength) += sizeOfSerialMPZ(commitStruct -> k_boxes_0[i][j] -> r);
				(*outputLength) += sizeOfSerialMPZ(commitStruct -> k_boxes_1[i][j] -> r);
			}
		}
		else
		{
			for(j = 0; j < commitStruct -> numInputs; j ++)
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

	for(i = 0; i < commitStruct -> numCircuits; i ++)
	{
		if(0x01 == jSetPartner[i])
		{
			memcpy(outputBuffer + bufferOffset, commitStruct -> commitmentPerms[i], tempBytesCount);
			bufferOffset += tempBytesCount;

			for(j = 0; j < commitStruct -> numInputs; j ++)
			{
				serialiseMPZ(commitStruct -> k_boxes_0[i][j] -> r, outputBuffer, &bufferOffset);
				serialiseMPZ(commitStruct -> k_boxes_1[i][j] -> r, outputBuffer, &bufferOffset);
			}
		}
	}

	for(i = 0; i < commitStruct -> numCircuits; i ++)
	{
		if(0x00 == jSetPartner[i])
		{
			for(j = 0; j < commitStruct -> numInputs; j ++)
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
	unsigned char permedBit, **permedBits;
	unsigned char **permutations = (unsigned char **) calloc(numCircuits, sizeof(unsigned char *)), *x0, *x1;
	int i, j, k, tempOffset = *inputOffset, validCommitments0 = 0, validCommitments1 = 0;
	int tempBytesCount = numInputs * sizeof(unsigned char);
	mpz_t *tempR0, *tempR1;
	mpz_t **tempR0List, **tempR1List, **tempR_List;


	tempR0List = (mpz_t **) calloc(numCircuits, sizeof(mpz_t *));
	tempR1List = (mpz_t **) calloc(numCircuits, sizeof(mpz_t *));
	tempR_List = (mpz_t **) calloc(numCircuits, sizeof(mpz_t *));

	for(i = 0; i < numCircuits; i ++)
	{
		if(0x01 == jSetOwn[i])
		{
			tempR0List[i] = (mpz_t *) calloc(numInputs, sizeof(mpz_t));
			tempR1List[i] = (mpz_t *) calloc(numInputs, sizeof(mpz_t));

			permutations[i] = (unsigned char *) calloc(numInputs, sizeof(unsigned char));
			memcpy(permutations[i], inputBuffer + tempOffset, tempBytesCount);
			tempOffset += tempBytesCount;

			for(j = 0; j < numInputs; j ++)
			{
				tempR0 = deserialiseMPZ(inputBuffer, &tempOffset);
				mpz_init_set(tempR0List[i][j], *tempR0);

				tempR1 = deserialiseMPZ(inputBuffer, &tempOffset);
				mpz_init_set(tempR1List[i][j], *tempR1);

				mpz_clear(*tempR0);
				free(tempR0);
				mpz_clear(*tempR1);
				free(tempR1);
			}
		}
	}

	// This could be optimised by removing the branching.
	#pragma omp parallel for default(shared) private(i, j, k, x0, x1) schedule(auto) reduction(|:validCommitments0)
	for(i = 0; i < numCircuits; i ++)
	{
		if(0x01 == jSetOwn[i])
		{
			k = 0;

			for(j = 0; j < numInputs; j ++)
			{
				x0 = hashECC_Point(NaorPinkasInputs_Partner[i][k], 16);
				k ++;
				x1 = hashECC_Point(NaorPinkasInputs_Partner[i][k], 16);
				k ++;

				if(0x00 == permutations[i][j])
				{
					validCommitments0 |= single_decommit_raw_elgamal_R(commitStruct -> params, commitStruct -> c_boxes_0[i][j],
																	x0, tempR0List[i][j], 16);
					validCommitments0 |= single_decommit_raw_elgamal_R(commitStruct -> params, commitStruct -> c_boxes_1[i][j],
																	x1, tempR1List[i][j], 16);
				}
				else
				{
					validCommitments0 |= single_decommit_raw_elgamal_R(commitStruct -> params, commitStruct -> c_boxes_0[i][j],
																	x1, tempR0List[i][j], 16);
					validCommitments0 |= single_decommit_raw_elgamal_R(commitStruct -> params, commitStruct -> c_boxes_1[i][j], 
																	x0, tempR1List[i][j], 16);
				}

				mpz_clear(tempR0List[i][j]);
				mpz_clear(tempR1List[i][j]);
				free(x0);
				free(x1);
			}
		}
	}


	permedBits = (unsigned char **) calloc(numCircuits, sizeof(unsigned char *));
	// Now check that the 
	for(i = 0; i < numCircuits; i ++)
	{
		if(0x00 == jSetOwn[i])
		{
			tempR_List[i] = (mpz_t *) calloc(numInputs, sizeof(mpz_t));
			permedBits[i] = (unsigned char *) calloc(numInputs, sizeof(unsigned char));
			for(j = 0; j < numInputs; j ++)
			{
				permedBits[i][j] = inputBuffer[tempOffset ++];

				// x0 = hashECC_Point(partnerReveals -> builderInputsEval[i][j], 16);
				tempR0 = deserialiseMPZ(inputBuffer, &tempOffset);
				mpz_init_set(tempR_List[i][j], *tempR0);

				mpz_clear(*tempR0);
				free(tempR0);
			}
		}
	}

	#pragma omp parallel for default(shared) private(i, j, x0) schedule(auto) reduction(|:validCommitments1)
	for(i = 0; i < numCircuits; i ++)
	{
		if(0x00 == jSetOwn[i])
		{
			for(j = 0; j < numInputs; j ++)
			{
				x0 = hashECC_Point(partnerReveals -> builderInputsEval[i][j], 16);
				if(0x00 == permedBits[i][j])
				{
					validCommitments1 |= single_decommit_raw_elgamal_R(commitStruct -> params, commitStruct -> c_boxes_0[i][j],
																	x0, tempR_List[i][j], 16);
				}
				else
				{
					validCommitments1 |= single_decommit_raw_elgamal_R(commitStruct -> params, commitStruct -> c_boxes_1[i][j],
																	x0, tempR_List[i][j], 16);
				}
			}
		}
	}


	*inputOffset = tempOffset;


	return (validCommitments0 | validCommitments1);
}