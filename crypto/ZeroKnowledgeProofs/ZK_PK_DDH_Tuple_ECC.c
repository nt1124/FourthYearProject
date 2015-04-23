struct witnessStruct *proverSetupWitnesses(int stat_SecParam, mpz_t *alphas_List)
{
	struct witnessStruct *witnessSet = initWitnessStruc(stat_SecParam);
	int i, j = 0;


	for(i = 0; i < stat_SecParam; i ++)
	{
		mpz_set(witnessSet -> witnesses[i], alphas_List[i]);
		j++;
	}

	return witnessSet;
}


struct alphaAndA_Struct *proverSetupCommitment_ECC(struct eccParams *params, struct eccPoint *g_0, gmp_randstate_t state)
{
	struct alphaAndA_Struct *alphaAndA = (struct alphaAndA_Struct*) calloc(1, sizeof(struct alphaAndA_Struct));


	mpz_init(alphaAndA -> a);

	mpz_urandomm(alphaAndA -> a, state, params -> n);
	alphaAndA -> alpha = windowedScalarPoint(alphaAndA -> a, g_0, params);

	return alphaAndA;
}


struct verifierCommitment_ECC *verifierSetupCommitment_ECC(struct eccParams *params, struct eccPoint *g_0,
															struct eccPoint *alpha, gmp_randstate_t state)
{
	struct verifierCommitment_ECC *commitment_box = initVerifierCommitment_ECC();
	struct eccPoint *temp1, *temp2;


	mpz_urandomm(commitment_box -> t, state, params -> n);
	mpz_urandomm(commitment_box -> c, state, params -> n);

	temp1 = windowedScalarPoint(commitment_box -> c, g_0, params);
	temp2 = windowedScalarPoint(commitment_box -> t, alpha, params);

	commitment_box -> C_commit = groupOp(temp1, temp2, params);


	return commitment_box;
}

	
struct msgOneArrays_ECC *proverMessageOne_ECC(int stat_SecParam, struct eccParams *params,
											unsigned char *J_set, struct eccPoint *alpha,
											struct eccPoint *g_0, struct eccPoint *g_1,
											struct eccPoint **h_0_List, struct eccPoint **h_1_List, gmp_randstate_t state)
{
	struct msgOneArrays_ECC *msgArray = initMsgOneArray_ECC(stat_SecParam);
	struct eccPoint *topHalf, *bottomHalf;
	int i, j_in_I = 0, j_not_I = 0;


	for(i = 0; i < stat_SecParam; i ++)
	{
		// If i IS in I
		if(0x00 == J_set[i])
		{
			msgArray -> notI_Struct -> not_in_I_index[j_not_I] = i + 1;

			mpz_urandomm(msgArray -> notI_Struct -> C_array[j_not_I], state, params -> n);
			mpz_urandomm(msgArray -> notI_Struct -> Z_array[j_not_I], state, params -> n);

			topHalf = windowedScalarPoint(msgArray -> notI_Struct -> Z_array[j_not_I], g_0, params);
			bottomHalf = windowedScalarPoint(msgArray -> notI_Struct -> C_array[j_not_I], h_0_List[i], params);
			msgArray -> A_array[i] = invertPoint(bottomHalf, params);
			groupOp_PlusEqual(msgArray -> A_array[i], topHalf, params);

			clearECC_Point(topHalf);
			clearECC_Point(bottomHalf);

			topHalf = windowedScalarPoint(msgArray -> notI_Struct -> Z_array[j_not_I], g_1, params);
			bottomHalf = windowedScalarPoint(msgArray -> notI_Struct -> C_array[j_not_I], h_1_List[i], params);
			msgArray -> B_array[i] = invertPoint(bottomHalf, params);
			groupOp_PlusEqual(msgArray -> B_array[i], topHalf, params);			

			clearECC_Point(topHalf);
			clearECC_Point(bottomHalf);

			j_not_I ++;
		}
		else
		{
			msgArray -> in_I_index[j_in_I] = i + 1;

			mpz_urandomm(msgArray -> roeArray[j_in_I], state, params -> n);

			msgArray -> A_array[i] = windowedScalarPoint(msgArray -> roeArray[j_in_I], g_0, params);
			msgArray -> B_array[i] = windowedScalarPoint(msgArray -> roeArray[j_in_I], g_1, params);

			j_in_I ++;
		}
	}


	return msgArray;
}


unsigned char *verifierQuery_ECC(struct verifierCommitment_ECC *commitment_box, int *outputLen)
{
	unsigned char *commBuffer;
	int totalLength = 2 * sizeof(int), bufferOffset = 0;


	totalLength += sizeof(mp_limb_t) * (mpz_size(commitment_box -> c) + mpz_size(commitment_box -> t));
	commBuffer = (unsigned char *) calloc(totalLength, sizeof(unsigned char));

	serialiseMPZ(commitment_box -> c, commBuffer, &bufferOffset);
	serialiseMPZ(commitment_box -> t, commBuffer, &bufferOffset);

	*outputLen = bufferOffset;


	return commBuffer;
}


int checkC_prover_ECC(struct eccParams *params, struct eccPoint *g_0, struct verifierCommitment_ECC *commitment_box, struct eccPoint *alpha)
{
	struct eccPoint *checkC, *temp1;
	int i, checkC_Correct = 0;


	// Check the C = (g_0)^c * (alpha)^t
	temp1 = windowedScalarPoint(commitment_box -> c, g_0, params);
	checkC = windowedScalarPoint(commitment_box -> t, alpha, params);
	groupOp_PlusEqual(checkC, temp1, params);


	checkC_Correct = eccPointsEqual(checkC, commitment_box -> C_commit);

	clearECC_Point(checkC);
	clearECC_Point(temp1);


	return checkC_Correct;
}


unsigned char *computeAndSerialise_ECC(struct eccParams *params, int stat_SecParam, unsigned char *J_set,
									struct msgOneArrays_ECC *msgArray, struct witnessStruct *witnessesArray,
									mpz_t *cShares, struct alphaAndA_Struct *alphaAndA, int *outputLen)
{
	unsigned char *commBuffer, *zBuffer, *cBuffer, *aBuffer;
	mpz_t temp1, temp2, *z_ToSend, *temp;
	int i, j_in_I = 0, j_not_I = 0;
	int bufferOffset = sizeof(int), totalLength;
	int zLength = 0, cLength = 0, aLength = 0;


	mpz_init(temp1);
	mpz_init(temp2);

	totalLength = (2 * stat_SecParam + 3) * sizeof(int);
	z_ToSend = (mpz_t*) calloc(stat_SecParam, sizeof(mpz_t));

	for(i = 0; i < stat_SecParam; i ++)
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

	zBuffer = serialiseMPZ_Array(z_ToSend, stat_SecParam, &zLength);
	cBuffer = serialiseMPZ_Array(cShares, stat_SecParam, &cLength);
	aBuffer = (unsigned char*) calloc(sizeof(int) + (mpz_size(alphaAndA -> a) * sizeof(mp_limb_t)), sizeof(unsigned char));
	serialiseMPZ(alphaAndA -> a, aBuffer, &aLength);

	commBuffer = (unsigned char *) calloc(zLength + cLength + aLength, sizeof(unsigned char));

	memcpy(commBuffer, zBuffer, zLength);
	memcpy(commBuffer + zLength, cBuffer, cLength);
	memcpy(commBuffer + zLength + cLength, aBuffer, aLength);

	*outputLen = zLength + cLength + aLength;


	return commBuffer;
}


unsigned char *proverMessageTwo_ECC(struct eccParams *params, int stat_SecParam, unsigned char *J_set,
								struct verifierCommitment_ECC *commitment_box, struct eccPoint *g_0,
								struct msgOneArrays_ECC *msgArray, struct witnessStruct *witnessesArray,
								struct alphaAndA_Struct *alphaAndA, int *outputLen)
{
	unsigned char *commBuffer;
	mpz_t *cShares, *tempPointer;
	int i, checkC_Correct = 0, bufferOffset = 0;
	int *tempDeltaI = (int*) calloc(stat_SecParam, sizeof(int));
	int tempIndex = 0;


	checkC_Correct = checkC_prover_ECC(params, g_0, commitment_box, alphaAndA -> alpha);
	if(0 != checkC_Correct)
	{
		return NULL;
	}

	for(i = 0; i < stat_SecParam; i ++)
	{
		tempDeltaI[i] = i + 1;
	}

	// Here the secret sharing distrbution happens. Reed-Solomon etc.
	if(2 != stat_SecParam)
	{
		cShares = completePartialSecretSharing(msgArray -> notI_Struct -> not_in_I_index, msgArray -> notI_Struct -> C_array,
												commitment_box -> c, params -> n, stat_SecParam);
	}
	else
	{
		cShares = (mpz_t *) calloc(2, sizeof(mpz_t));
		tempIndex = msgArray -> notI_Struct -> not_in_I_index[0] - 1;
		mpz_init(cShares[1 - tempIndex]);
		mpz_init_set(cShares[tempIndex], msgArray -> notI_Struct -> C_array[0]);
		mpz_add(cShares[1 - tempIndex], cShares[tempIndex], commitment_box -> c);
	}


	commBuffer = computeAndSerialise_ECC(params, stat_SecParam, J_set, msgArray,
										witnessesArray, cShares, alphaAndA, &bufferOffset);


	*outputLen = bufferOffset;
	
	return commBuffer;
}


int verifierChecks_ECC(struct eccParams *params, int stat_SecParam,
					struct eccPoint *g_0, struct eccPoint *g_1,
					struct eccPoint **h_0_List, struct eccPoint **h_1_List,
					mpz_t *Z_array, struct eccPoint **A_array, struct eccPoint **B_array,
					struct alphaAndA_Struct *alphaAndA, mpz_t *codewords, mpz_t c)
{
	struct Fq_poly *secretPoly;
	struct eccPoint *A_check, *B_check, *alpha;
	struct eccPoint *topHalf, *bottomHalf;

	mpz_t cShares2Check, cShares2CheckAbs;
	int *delta_i = (int*) calloc(stat_SecParam, sizeof(int*));
	int i, alphaCheck = 0, AB_check = 0, finalDecision = 0;



	A_check = (struct eccPoint*) calloc(1, sizeof(struct eccPoint));
	B_check = (struct eccPoint*) calloc(1, sizeof(struct eccPoint));


	alpha = windowedScalarPoint(alphaAndA -> a, g_0, params);
	alphaCheck = eccPointsEqual(alpha, alphaAndA -> alpha);


	for(i = 0; i < stat_SecParam; i ++)
	{
		delta_i[i] = i + 1;
	}


	if(2 != stat_SecParam)
	{
		secretPoly = getPolyFromCodewords(codewords, delta_i, stat_SecParam / 2 + 1, params -> n);
		finalDecision = testSecretScheme(secretPoly, c, params -> n, (stat_SecParam / 2) + 1, stat_SecParam);
	}
	else
	{
		mpz_init(cShares2Check);
		mpz_init(cShares2CheckAbs);

		mpz_sub(cShares2Check, codewords[0], codewords[1]);
		mpz_abs(cShares2CheckAbs, cShares2Check);

		finalDecision = mpz_cmp(c, cShares2CheckAbs);
	}


	for(i = 0; i < stat_SecParam; i ++)
	{
		topHalf = windowedScalarPoint(Z_array[i], g_0, params);
		bottomHalf = windowedScalarPoint(codewords[i], h_0_List[i], params);
		A_check = invertPoint(bottomHalf, params);
		groupOp_PlusEqual(A_check, topHalf, params);
		
		clearECC_Point(topHalf);
		clearECC_Point(bottomHalf);


		topHalf = windowedScalarPoint(Z_array[i], g_1, params);
		bottomHalf = windowedScalarPoint(codewords[i], h_1_List[i], params);
		B_check = invertPoint(bottomHalf, params);
		groupOp_PlusEqual(B_check, topHalf, params);
		
		clearECC_Point(topHalf);
		clearECC_Point(bottomHalf);

		AB_check |= eccPointsEqual(A_array[i], A_check);
		AB_check |= eccPointsEqual(B_array[i], B_check);

		clearECC_Point(A_check);
		clearECC_Point(B_check);
	}

	finalDecision |= AB_check;
	finalDecision |= alphaCheck;

	return finalDecision;
}



void ZKPoK_Prover_ECC(int writeSocket, int readSocket, 	struct eccParams *params, int stat_SecParam,
					struct eccPoint *g_0, struct eccPoint *g_1,
					struct eccPoint **h_0_List, struct eccPoint **h_1_List,
					mpz_t *alphas_List, unsigned char *J_set, gmp_randstate_t *state)
{
	struct witnessStruct *witnessSet;
	mpz_t *tempMPZ;
	struct alphaAndA_Struct *alphaAndA_P;
	struct verifierCommitment_ECC *commitment_box_P;
	struct msgOneArrays_ECC *msgOne_P;

	unsigned char *commBuffer;
	int bufferOffset = 0, commBufferLen = 0;


	witnessSet = proverSetupWitnesses(stat_SecParam, alphas_List);
	alphaAndA_P = proverSetupCommitment_ECC(params, g_0, *state);

	bufferOffset = 0;
	commBuffer = (unsigned char *) calloc(sizeOfSerial_ECCPoint(alphaAndA_P -> alpha), sizeof(unsigned char));
	serialise_ECC_Point(alphaAndA_P -> alpha, commBuffer, &bufferOffset);
	sendBoth(writeSocket, commBuffer, bufferOffset);
	// Round 1


	bufferOffset = 0;
	commBufferLen = 0;
	commitment_box_P = initVerifierCommitment_ECC();


	commBuffer = receiveBoth(readSocket, commBufferLen);
	commitment_box_P -> C_commit = deserialise_ECC_Point(commBuffer, &bufferOffset);
	free(commBuffer);
	// Round 2


	bufferOffset = 0;
	msgOne_P = proverMessageOne_ECC(stat_SecParam, params, J_set, alphaAndA_P -> alpha, g_0, g_1,
									h_0_List, h_1_List, *state);

	commBuffer = serialise_A_B_Arrays_ECC(msgOne_P, stat_SecParam, &bufferOffset);
	sendBoth(writeSocket, commBuffer, bufferOffset);
	// Round 3


	bufferOffset = 0;
	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	deserialisedSecrets_CommitmentBox(commitment_box_P, commBuffer, &bufferOffset);
	free(commBuffer);
	// Round 4


	bufferOffset = 0;
	commBuffer = proverMessageTwo_ECC(params, stat_SecParam, J_set, commitment_box_P, g_0, msgOne_P, witnessSet, alphaAndA_P, &bufferOffset);
	sendBoth(writeSocket, commBuffer, bufferOffset);
	// Round 5
}



int ZKPoK_Verifier_ECC(int writeSocket, int readSocket, struct eccParams *params, int stat_SecParam,
						struct eccPoint *g_0, struct eccPoint *g_1,
						struct eccPoint **h_0_List, struct eccPoint **h_1_List,
						gmp_randstate_t *state)
{
	struct alphaAndA_Struct *alphaAndA_V = (struct alphaAndA_Struct *) calloc(1, sizeof(struct alphaAndA_Struct));
	struct verifierCommitment_ECC *commitment_box_V;
	struct msgOneArrays_ECC *msgOne_V;
	mpz_t *tempMPZ, *Z_array_V, *cShares_V;
	unsigned char *commBuffer;

	int cCheck = 0, verified = 0, bufferOffset = 0, commBufferLen = 0, i;


	bufferOffset = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	alphaAndA_V -> alpha = deserialise_ECC_Point(commBuffer, &bufferOffset);
	free(commBuffer);
	// Round 1


	commitment_box_V = verifierSetupCommitment_ECC(params, g_0, alphaAndA_V -> alpha, *state);

	bufferOffset = 0;
	commBuffer = (unsigned char *) calloc(sizeOfSerial_ECCPoint(commitment_box_V -> C_commit), sizeof(unsigned char));
	serialise_ECC_Point(commitment_box_V -> C_commit, commBuffer, &bufferOffset);
	sendBoth(writeSocket, commBuffer, bufferOffset);
	// Round 2

	msgOne_V = initMsgOneArray_ECC(stat_SecParam);
	bufferOffset = 0;
	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);

	for(i = 0; i < stat_SecParam; i ++)
	{
		msgOne_V -> A_array[i] = deserialise_ECC_Point(commBuffer, &bufferOffset);
		msgOne_V -> B_array[i] = deserialise_ECC_Point(commBuffer, &bufferOffset);
	}
	free(commBuffer);
	// Round 3

	bufferOffset = 0;
	commBuffer = verifierQuery_ECC(commitment_box_V, &bufferOffset);
	sendBoth(writeSocket, commBuffer, bufferOffset);
	// Round 4


	bufferOffset = 0;
	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);

	Z_array_V = deserialiseMPZ_Array(commBuffer, &bufferOffset);
	cShares_V = deserialiseMPZ_Array(commBuffer, &bufferOffset);
	tempMPZ = deserialiseMPZ(commBuffer, &bufferOffset);
	mpz_set(alphaAndA_V -> a, *tempMPZ);


	verified = verifierChecks_ECC(params, stat_SecParam, g_0, g_1, h_0_List, h_1_List, Z_array_V,
								msgOne_V -> A_array, msgOne_V -> B_array, alphaAndA_V, cShares_V, commitment_box_V -> c);

	return verified;
}



void test_ZKPoK_ECC()
{
	struct params_CnC_ECC *params_P,  *params_V;
	struct witnessStruct *witnessSet;
	struct alphaAndA_Struct *alphaAndA_P, *alphaAndA_V;
	mpz_t *tempMPZ;
	struct verifierCommitment_ECC *commitment_box_P, *commitment_box_V;
	struct msgOneArrays_ECC *msgOne_P, *msgOne_V;

	unsigned char *commBuffer;
	unsigned char sigmaBit = 0x00, *J_set;

	int i, j, k, numTests = 2, comp_SecParam = 102, cCheck = 0, verified = 0;
	int bufferOffset = 0, u_v_index = 0, tempInt = 0;

	gmp_randstate_t *state = seedRandGen();


	J_set = generateJ_Set(stat_SecParam);
	params_P = setup_CnC_OT_Receiver_ECC(numTests, J_set, comp_SecParam, *state);
	commBuffer = serialiseParams_CnC_ECC(params_P, &bufferOffset);
	params_V = setup_CnC_OT_Sender_ECC(commBuffer);
	free(commBuffer);


	witnessSet = proverSetupWitnesses(params_P -> crs -> stat_SecParam, params_P -> crs -> alphas_List);
	alphaAndA_P = proverSetupCommitment_ECC(params_P -> params, params_P -> params -> g, *state);


	bufferOffset = 0;
	commBuffer = (unsigned char *) calloc(sizeOfSerial_ECCPoint(alphaAndA_P -> alpha), sizeof(unsigned char));
	serialise_ECC_Point(alphaAndA_P -> alpha, commBuffer, &bufferOffset);


	bufferOffset = 0;
	alphaAndA_V = (struct alphaAndA_Struct*) calloc(1, sizeof(struct alphaAndA_Struct));
	alphaAndA_V -> alpha = deserialise_ECC_Point(commBuffer, &bufferOffset);
	free(commBuffer);


	commitment_box_V = verifierSetupCommitment_ECC(params_V -> params, params_V -> params -> g,
													alphaAndA_V -> alpha, *state);

	bufferOffset = 0;
	commBuffer = (unsigned char *) calloc(sizeOfSerial_ECCPoint(commitment_box_V -> C_commit), sizeof(unsigned char));
	serialise_ECC_Point(commitment_box_V -> C_commit, commBuffer, &bufferOffset);


	commitment_box_P = initVerifierCommitment_ECC();
	bufferOffset = 0;
	commitment_box_P -> C_commit = deserialise_ECC_Point(commBuffer, &bufferOffset);
	free(commBuffer);


	bufferOffset = 0;
	msgOne_P = proverMessageOne_ECC(params_P -> crs -> stat_SecParam, params_P -> params,
									params_P -> crs -> J_set, alphaAndA_P -> alpha,
									params_P -> params -> g, params_P -> crs -> g_1,
									params_P -> crs -> h_0_List, params_P -> crs -> h_1_List, *state);
	commBuffer = serialise_A_B_Arrays_ECC(msgOne_P, params_P -> crs -> stat_SecParam, &bufferOffset);


	msgOne_V = initMsgOneArray_ECC(params_V -> crs -> stat_SecParam);
	bufferOffset = 0;
	for(i = 0; i < params_V -> crs -> stat_SecParam; i ++)
	{
		msgOne_V -> A_array[i] = deserialise_ECC_Point(commBuffer, &bufferOffset);
		msgOne_V -> B_array[i] = deserialise_ECC_Point(commBuffer, &bufferOffset);
	}
	free(commBuffer);


	bufferOffset = 0;
	commBuffer = verifierQuery_ECC(commitment_box_V, &bufferOffset);


	bufferOffset = 0;
	deserialisedSecrets_CommitmentBox(commitment_box_P, commBuffer, &bufferOffset);
	free(commBuffer);


	bufferOffset = 0;
	commBuffer = proverMessageTwo_ECC(params_P -> params, params_P -> crs -> stat_SecParam,params_P -> crs -> J_set,
									commitment_box_P, params_P -> params -> g, msgOne_P, witnessSet, alphaAndA_P, &bufferOffset);


	bufferOffset = 0;
	mpz_t *Z_array_V = deserialiseMPZ_Array(commBuffer, &bufferOffset);
	mpz_t *cShares_V = deserialiseMPZ_Array(commBuffer, &bufferOffset);
	tempMPZ = deserialiseMPZ(commBuffer, &bufferOffset);
	mpz_set(alphaAndA_V -> a, *tempMPZ);


	// struct eccParams *params, int stat_SecParam, struct eccPoint *g_1, struct eccPoint **h_0_List, struct eccPoint **h_1_List,
	verified = verifierChecks_ECC(params_V -> params, params_V -> crs -> stat_SecParam,
								params_V -> params -> g,params_V -> crs -> g_1,
								params_V -> crs -> h_0_List, params_V -> crs -> h_1_List,
								Z_array_V, msgOne_V -> A_array, msgOne_V -> B_array,
								alphaAndA_V, cShares_V, commitment_box_V -> c);

	printf("%d\n",verified);
}
