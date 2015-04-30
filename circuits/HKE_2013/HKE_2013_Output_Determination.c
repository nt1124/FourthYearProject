mpz_t *constructSharesMPZ_FromCircuits(mpz_t wireShare, mpz_t **outputWireShares, int j, unsigned char *J_SetOwn, unsigned char outputBit, int numCircuits)
{
	mpz_t *tempMPZ = (mpz_t *) calloc(1, sizeof(mpz_t)), *sharesToUse;
	int i, k = 2 * j + outputBit, h = 0;


	sharesToUse = (mpz_t *) calloc((numCircuits / 2) + 1, sizeof(mpz_t));
	for(i = 0; i < numCircuits; i ++)
	{
		if(0x01 == J_SetOwn[i])
		{
			mpz_init_set(sharesToUse[h ++], outputWireShares[i][k]);
		}
	}
	mpz_init_set(sharesToUse[h], wireShare);


	return sharesToUse;
}


// We put in shares in the order {J_set || current circuit} (|| here is concat)
int *getBaseIndiciesForShares(unsigned char *J_SetOwn, int numCircuits)
{
	int *indices = (int *) calloc((numCircuits / 2) + 1, sizeof(int)), i, j = 0;


	for(i = 0; i < numCircuits; i ++)
	{
		if(0x01 == J_SetOwn[i])
		{
			indices[j ++] = i + 1;
		}
	}


	return indices;
}


mpz_t **getEvalCircuitOutputShares(struct RawCircuit *rawInputCircuit, struct Circuit **circuitsArray_Partner, struct DDH_Group *group,
								struct HKE_Output_Struct_Builder *outputStruct_Partner, mpz_t **outputWireShares, gmp_randstate_t *state,
								unsigned char *J_SetOwn, int numCircuits, int numOutputs)
{
	unsigned char **trueOutputs = (unsigned char **) calloc(numCircuits, sizeof(unsigned char *));
	mpz_t **outputKeySecrets = (mpz_t **) calloc(numOutputs, sizeof(mpz_t *));
	mpz_t *tempMPZ = (mpz_t *) calloc(1, sizeof(mpz_t)), *revealedSecret, *sharesMPZ;
	int i, j, foundKey0, foundKey1, gateIndexOffset, *indices, verified = 0;
	struct wire *tempWire;


	mpz_init(*tempMPZ);
	for(j = 0; j < numCircuits; j ++)
	{
		if(0x00 == J_SetOwn[j])
		{
			trueOutputs[j] = getOutputAsBinary(circuitsArray_Partner[j], &gateIndexOffset);
		}
	}


	indices = getBaseIndiciesForShares(J_SetOwn, numCircuits);
	gateIndexOffset = rawInputCircuit -> numGates - rawInputCircuit -> numOutputs;


	for(i = 0; i < numOutputs; i ++)
	{
		j = 0;
		foundKey0 = 0;
		foundKey1 = 0;
		outputKeySecrets[i] = (mpz_t *) calloc(2, sizeof(mpz_t));
		mpz_init(outputKeySecrets[i][0]);
		mpz_init(outputKeySecrets[i][1]);

		while( j < numCircuits && (0 == foundKey0 || 0 == foundKey1) )
		{
			if(0x00 == J_SetOwn[j])
			{
				if(0 == foundKey0 && 0 == trueOutputs[j][i])
				{
					foundKey0 = j + 1;
					tempWire = circuitsArray_Partner[j] -> gates[gateIndexOffset + i] -> outputWire;
					convertBytesToMPZ(tempMPZ, tempWire -> wireOutputKey, 16);
					
					// gmp_printf("%d - %d - 0 - %Zd\n", j, i, *tempMPZ);
					sharesMPZ = constructSharesMPZ_FromCircuits(*tempMPZ, outputWireShares, i, J_SetOwn, 0x00, numCircuits);
					
					verified |= VSS_Verify(outputStruct_Partner -> scheme0Array[i] -> pub, *tempMPZ, j + 1, group);

					indices[(numCircuits / 2)] = j + 1;
					tempMPZ = VSS_Recover(sharesMPZ, indices, (numCircuits / 2) + 1, group);
					mpz_set(outputKeySecrets[i][0], *tempMPZ);
				}
				else if(0 == foundKey1 && 1 == trueOutputs[j][i])
				{
					foundKey1 = j + 1;
					tempWire = circuitsArray_Partner[j] -> gates[gateIndexOffset + i] -> outputWire;
					convertBytesToMPZ(tempMPZ, tempWire -> wireOutputKey, 16);

					// gmp_printf("%d - %d - 1 - %Zd\n", j, i, *tempMPZ);
					sharesMPZ = constructSharesMPZ_FromCircuits(*tempMPZ, outputWireShares, i, J_SetOwn, 0x01, numCircuits);

					verified |= VSS_Verify(outputStruct_Partner -> scheme1Array[i] -> pub, *tempMPZ, j + 1, group);

					indices[(numCircuits / 2)] = j + 1;
					tempMPZ = VSS_Recover(sharesMPZ, indices, (numCircuits / 2) + 1, group);
					mpz_set(outputKeySecrets[i][1], *tempMPZ);
				}
			}

			j ++;
		}

		if(0 == foundKey0)
		{
			mpz_urandomm(outputKeySecrets[i][0], *state, group -> q);
		}
		// gmp_printf("%d - %Zd\n", i, outputKeySecrets[i][0]);
		if(0 == foundKey1)
		{
			mpz_urandomm(outputKeySecrets[i][1], *state, group -> q);
		}
		// gmp_printf("%d - %Zd\n", i, outputKeySecrets[i][1]);

	}

	printf("verified = %d\n", verified);

	return outputKeySecrets;
}


unsigned char **getSecureEqualityTestInputs(struct HKE_Output_Struct_Builder *outputStruct_Own, mpz_t **potentialSecrets, int numOutputs)
{
	unsigned char **secureEqualityInputs = (unsigned char **) calloc(2 * numOutputs, sizeof(unsigned char *));
	unsigned char *ownSecretChars, *partnerSecretChars;
	int i, j = 0, ownLength, partnerLength, h;

	for(i = 0; i < numOutputs; i ++)
	{
		ownSecretChars = convertMPZToBytes(outputStruct_Own -> scheme0Array[i] -> secret, &ownLength);
		partnerSecretChars = convertMPZToBytes(potentialSecrets[i][0], &partnerLength);
		secureEqualityInputs[j] = XOR_StringsWithMinLen(ownSecretChars, partnerSecretChars, ownLength, partnerLength, 128);
		// XOR_TwoStringsDiffLength(ownSecretChars, partnerSecretChars, ownLength, partnerLength);
		j ++;

		ownSecretChars = convertMPZToBytes(outputStruct_Own -> scheme1Array[i] -> secret, &ownLength);
		partnerSecretChars = convertMPZToBytes(potentialSecrets[i][1], &partnerLength);
		secureEqualityInputs[j] = XOR_StringsWithMinLen(ownSecretChars, partnerSecretChars, ownLength, partnerLength, 128);
		// XOR_TwoStringsDiffLength(ownSecretChars, partnerSecretChars, ownLength, partnerLength);
		j ++;
	}

	return secureEqualityInputs;
}



// Should just use the normal global
struct secureEqualityCommitments *commitForEqualityTest(unsigned char **secureEqualityInputs, gmp_randstate_t *state, int numOutputs)
{
	struct secureEqualityCommitments *outputStruct = (struct secureEqualityCommitments *) calloc(1, sizeof(struct secureEqualityCommitments));
	int i, j = 0;


	outputStruct -> c_boxes_0 = (struct elgamal_commit_box **) calloc(numOutputs, sizeof(struct elgamal_commit_box *));
	outputStruct -> c_boxes_1 = (struct elgamal_commit_box **) calloc(numOutputs, sizeof(struct elgamal_commit_box *));
	outputStruct -> k_boxes_0 = (struct elgamal_commit_key **) calloc(numOutputs, sizeof(struct elgamal_commit_key *));
	outputStruct -> k_boxes_1 = (struct elgamal_commit_key **) calloc(numOutputs, sizeof(struct elgamal_commit_key *));

	outputStruct -> params = generate_commit_params(1024, *state);

	for(i = 0; i < numOutputs; i ++)
	{
		outputStruct -> c_boxes_0[i] = init_commit_box();
		outputStruct -> c_boxes_1[i] = init_commit_box();
			
		outputStruct -> k_boxes_0[i] = init_commit_key();
		outputStruct -> k_boxes_1[i] = init_commit_key();

		create_commit_box_key(outputStruct -> params, secureEqualityInputs[j ++], 128, *state, outputStruct -> c_boxes_0[i], outputStruct -> k_boxes_0[i]);
		create_commit_box_key(outputStruct -> params, secureEqualityInputs[j ++], 128, *state, outputStruct -> c_boxes_1[i], outputStruct -> k_boxes_1[i]);
	}


	return outputStruct;
}



unsigned char *serialiseC_Boxes_SecEqTest(struct secureEqualityCommitments *commitStruct, int numOutputs, int *totalLength)
{
	unsigned char *outputBuffer, *temp0, *temp1, *temp2;
	int outputLength = 0, tempOffset0, tempOffset1, tempOffset2, temp2Length;


	tempOffset0 = 0;
	temp0 = serialiseAllC_Boxes_1D(commitStruct -> c_boxes_0, numOutputs, &tempOffset0);
	outputLength += tempOffset0;

	tempOffset1 = 0;
	temp1 = serialiseAllC_Boxes_1D(commitStruct -> c_boxes_1, numOutputs, &tempOffset1);
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


struct secureEqualityCommitments *deserialiseC_Boxes_SecEqTest(unsigned char *inputBuffer, int numOutputs, int *inputOffset)
{
	struct secureEqualityCommitments *outputStruct = (struct secureEqualityCommitments *) calloc(1, sizeof(struct secureEqualityCommitments));
	int i, j, tempOffset = *inputOffset;


	// outputStruct -> c_boxes_0 = (struct elgamal_commit_box **) calloc(numOutputs, sizeof(struct elgamal_commit_box *));
	// outputStruct -> c_boxes_1 = (struct elgamal_commit_box **) calloc(numOutputs, sizeof(struct elgamal_commit_box *));

	outputStruct -> c_boxes_0 = deserialiseAllC_Boxes_1D(inputBuffer, numOutputs, &tempOffset);
	outputStruct -> c_boxes_1 = deserialiseAllC_Boxes_1D(inputBuffer, numOutputs, &tempOffset);


	outputStruct -> params = deserialise_commit_batch_params(inputBuffer, &tempOffset);


	return outputStruct;
}



unsigned char *decommitOwn_SecEqTest(struct secureEqualityCommitments *commitStruct_Own, int numOutputs,
									int *totalLength)
{
	unsigned char *outputBuffer;
	int i, j, outputLength = 0, tempLength0, tempLength1, tempOffset = 0;


	outputLength = 2 * numOutputs * sizeof(int);

	for(i = 0; i < numOutputs; i ++)
	{
		tempLength0 = sizeof(unsigned char) * commitStruct_Own -> k_boxes_0[i] -> xLen + sizeOfSerialMPZ(commitStruct_Own -> k_boxes_0[i] -> r);
		tempLength1 = sizeof(unsigned char) * commitStruct_Own -> k_boxes_1[i] -> xLen + sizeOfSerialMPZ(commitStruct_Own -> k_boxes_1[i] -> r);
		outputLength += (tempLength0 + tempLength1);
	}

	outputBuffer = (unsigned char *) calloc(outputLength, sizeof(unsigned char));

	for(i = 0; i < numOutputs; i ++)
	{
		tempLength0 = commitStruct_Own -> k_boxes_0[i] -> xLen * sizeof(unsigned char);
		serialiseMPZ(commitStruct_Own -> k_boxes_0[i] -> r, outputBuffer, &tempOffset);
		memcpy(outputBuffer + tempOffset, &(commitStruct_Own -> k_boxes_0[i] -> xLen), sizeof(int));
		tempOffset += sizeof(int);

		memcpy(outputBuffer + tempOffset, commitStruct_Own -> k_boxes_0[i] -> x, tempLength0);
		tempOffset += tempLength0;


		tempLength1 = commitStruct_Own -> k_boxes_1[i] -> xLen * sizeof(unsigned char);
		serialiseMPZ(commitStruct_Own -> k_boxes_1[i] -> r, outputBuffer, &tempOffset);
		memcpy(outputBuffer + tempOffset, &(commitStruct_Own -> k_boxes_1[i] -> xLen), sizeof(int));
		tempOffset += sizeof(int);

		memcpy(outputBuffer + tempOffset, commitStruct_Own -> k_boxes_1[i] -> x, tempLength1);
		tempOffset += tempLength1;
	}


	*totalLength = outputLength;

	return outputBuffer;
}


// Next, having sent the decommitment to our partner we receive and process the decommitment our partner gave us! Friends and sharing and stuff!
unsigned char *decommitOwn_SecEqTest(struct secureEqualityCommitments *commitStruct_Partner, unsigned char **secureEqualityInputs, unsigned char *inputBuffer,
									int numOutputs, int *bufferOffset)
{
	unsigned char *binaryOutput = (unsigned char *) calloc(numOutputs, sizeof(unsigned char)), *x0, *x1;
	mpz_t *tempMPZ;
	int i, j = 0, xLen0, xLen1, tempOffset = *bufferOffset, test0, test1;


	for(i = 0; i < numOutputs; i ++)
	{
		tempMPZ = deserialiseMPZ(inputBuffer, &tempOffset);
		memcpy(&xLen0, inputBuffer + tempOffset, sizeof(int));
		tempOffset += sizeof(int);

		x0 = (unsigned char *) calloc(xLen0, sizeof(unsigned char));
		memcpy(x0, inputBuffer + tempOffset, xLen0);
		test0 = single_decommit_raw_elgamal_R(commitStruct_Partner -> params, commitStruct_Partner -> c_boxes_0[i], inputBuffer + tempOffset, *tempMPZ, xLen0);
		tempOffset += xLen0;

		mpz_clear(*tempMPZ);
		free(tempMPZ);


		tempMPZ = deserialiseMPZ(inputBuffer, &tempOffset);
		memcpy(&xLen1, inputBuffer + tempOffset, sizeof(int));
		tempOffset += sizeof(int);

		x1 = (unsigned char *) calloc(xLen1, sizeof(unsigned char));
		memcpy(x1, inputBuffer + tempOffset, xLen1);
		test1 = single_decommit_raw_elgamal_R(commitStruct_Partner -> params, commitStruct_Partner -> c_boxes_1[i], inputBuffer + tempOffset, *tempMPZ, xLen1);
		tempOffset += xLen1;
		
		mpz_clear(*tempMPZ);
		free(tempMPZ);


		if(0 != test0 || 0 != test1)
		{
			printf("NOOOOO\n");
			return NULL;
		}
		else if(0 == memcmp(x0, secureEqualityInputs[j], xLen0))
		{
			binaryOutput[i] = 0x00;
			// printf("0");
		}
		else if(0 == memcmp(x1, secureEqualityInputs[j + 1], xLen1))
		{
			binaryOutput[i] = 0x01;
			// printf("1");
		}
		else
		{
			printf("NOOOOO\n");
			return NULL;
		}

		j += 2;
	}

	return binaryOutput;
}


unsigned char *HKE_OutputDetermination(int writeSocket, int readSocket, gmp_randstate_t *state, struct Circuit **circuitsArray_Partner, struct RawCircuit *rawInputCircuit,
										struct DDH_Group *groupPartner, struct jSetRevealHKE *partnerReveals,
										struct HKE_Output_Struct_Builder *outputStruct_Own, struct HKE_Output_Struct_Builder *outputStruct_Partner,
										int numCircuits, unsigned char *J_SetOwn, int *outputLength, int partyID)
{
	unsigned char *commBuffer, **secureEqualityInputs, *binaryOutputs, *hexOutputs;
	struct secureEqualityCommitments *secEqualityCommits_Own, *secEqualityCommits_Partner;

	struct OT_NP_Receiver_Query **queries_Own;
	mpz_t  **partnerSecretList;

	int bufferOffset = 0, commBufferLen;


	partnerSecretList = getEvalCircuitOutputShares(rawInputCircuit, circuitsArray_Partner, groupPartner,
												outputStruct_Partner, partnerReveals -> outputWireShares,
												state, J_SetOwn, numCircuits, rawInputCircuit -> numOutputs);

	secureEqualityInputs = getSecureEqualityTestInputs(outputStruct_Own, partnerSecretList, rawInputCircuit -> numOutputs);
	secEqualityCommits_Own = commitForEqualityTest(secureEqualityInputs, state, rawInputCircuit -> numOutputs);

	commBuffer = serialiseC_Boxes_SecEqTest(secEqualityCommits_Own, rawInputCircuit -> numOutputs, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);

	bufferOffset = 0;
	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	secEqualityCommits_Partner = deserialiseC_Boxes_SecEqTest(commBuffer, rawInputCircuit -> numOutputs, &bufferOffset);
	free(commBuffer);


	commBuffer = decommitOwn_SecEqTest(secEqualityCommits_Own, rawInputCircuit -> numOutputs, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);

	bufferOffset = 0;
	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	binaryOutputs = decommitOwn_SecEqTest(secEqualityCommits_Partner, secureEqualityInputs, commBuffer, rawInputCircuit -> numOutputs, &bufferOffset);


	// bufferOffset = 0;
	// hexOutputs = getOutputAsHex(binaryOutputs, rawInputCircuit -> numOutputs, &bufferOffset);
	// *outputLength = bufferOffset;
	*outputLength = rawInputCircuit -> numOutputs;

	return binaryOutputs;
}