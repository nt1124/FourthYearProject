
struct witnessStruct *proverSetupWitnesses_1Of2(mpz_t *alphas_List, unsigned char *J_set)
{
	struct witnessStruct *witnessSet = initWitnessStruc(2);
	int i;


	for(i = 0; i < 2; i ++)
	{
		mpz_set(witnessSet -> witnesses[i], alphas_List[i]);

	}

	return witnessSet;
}


struct witnessStruct *proverSetupWitnesses_1(mpz_t *alphas_List)
{
	struct witnessStruct *witnessSet = initWitnessStruc(2);
	int i;


	for(i = 0; i < 2; i ++)
	{
		mpz_set(witnessSet -> witnesses[i], *alphas_List);

	}

	return witnessSet;
}


struct alphaAndA_Struct *proverSetupCommitment_ECC_1Of2(struct eccParams *params, gmp_randstate_t state)
{
	struct alphaAndA_Struct *alphaAndA = (struct alphaAndA_Struct*) calloc(1, sizeof(struct alphaAndA_Struct));


	mpz_init(alphaAndA -> a);

	mpz_urandomm(alphaAndA -> a, state, params -> n);
	// alphaAndA -> alpha = windowedScalarPoint(alphaAndA -> a, params -> g, params);
	alphaAndA -> alpha = fixedPointMultiplication(gPreComputes, alphaAndA -> a, params);

	return alphaAndA;
}


struct verifierCommitment_ECC *verifierSetupCommitment_ECC_1Of2(struct eccParams *params,
															struct eccPoint *alpha, gmp_randstate_t state)
{
	struct verifierCommitment_ECC *commitment_box = initVerifierCommitment_ECC();
	struct eccPoint *temp1, *temp2;


	mpz_urandomm(commitment_box -> t, state, params -> n);
	mpz_urandomm(commitment_box -> c, state, params -> n);

	// temp1 = windowedScalarPoint(commitment_box -> c, params -> g, params);
	temp1 = fixedPointMultiplication(gPreComputes, commitment_box -> c, params);
	temp2 = windowedScalarPoint(commitment_box -> t, alpha, params);

	commitment_box -> C_commit = groupOp(temp1, temp2, params);


	return commitment_box;
}

	
struct msgOneArrays_ECC *proverMessageOne_ECC_1Of2(struct eccParams *params,
												unsigned char *J_set, struct eccPoint *alpha,
												struct eccPoint **g_0, struct eccPoint **g_1,
												struct eccPoint **h_0_List, struct eccPoint **h_1_List, gmp_randstate_t state)
{
	struct msgOneArrays_ECC *msgArray = initMsgOneArray_ECC(2);
	struct eccPoint *topHalf, *bottomHalf;
	int i, j_in_I = 0, j_not_I = 0;


	for(i = 0; i < 2; i ++)
	{
		// If i IS in I
		if(0x00 == J_set[i])
		{
			msgArray -> notI_Struct -> not_in_I_index[j_not_I] = i + 1;

			mpz_urandomm(msgArray -> notI_Struct -> C_array[j_not_I], state, params -> n);
			mpz_urandomm(msgArray -> notI_Struct -> Z_array[j_not_I], state, params -> n);
			

			topHalf = windowedScalarPoint(msgArray -> notI_Struct -> Z_array[j_not_I], g_0[i], params);
			bottomHalf = windowedScalarPoint(msgArray -> notI_Struct -> C_array[j_not_I], h_0_List[i], params);
			msgArray -> A_array[i] = invertPoint(bottomHalf, params);
			groupOp_PlusEqual(msgArray -> A_array[i], topHalf, params);

			clearECC_Point(topHalf);
			clearECC_Point(bottomHalf);


			topHalf = windowedScalarPoint(msgArray -> notI_Struct -> Z_array[j_not_I], g_1[i], params);
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

			msgArray -> A_array[i] = windowedScalarPoint(msgArray -> roeArray[j_in_I], g_0[i], params);
			msgArray -> B_array[i] = windowedScalarPoint(msgArray -> roeArray[j_in_I], g_1[i], params);

			j_in_I ++;
		}
	}


	return msgArray;
}


unsigned char *verifierQuery_ECC_1Of2(struct verifierCommitment_ECC *commitment_box, int *outputLen)
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


int checkC_prover_ECC_1Of2(struct eccParams *params, struct verifierCommitment_ECC *commitment_box, struct eccPoint *alpha)
{
	struct eccPoint *checkC, *temp1;
	int i, checkC_Correct = 0;


	// Check the C = (g_0)^c * (alpha)^t
	temp1 = windowedScalarPoint(commitment_box -> c, params -> g, params);
	checkC = windowedScalarPoint(commitment_box -> t, alpha, params);
	groupOp_PlusEqual(checkC, temp1, params);


	checkC_Correct = eccPointsEqual(checkC, commitment_box -> C_commit);

	clearECC_Point(checkC);
	clearECC_Point(temp1);


	return checkC_Correct;
}


unsigned char *computeAndSerialise_ECC_1Of2(struct eccParams *params, unsigned char *J_set,
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

	totalLength = (2 * 2 + 3) * sizeof(int);
	z_ToSend = (mpz_t*) calloc(2, sizeof(mpz_t));

	for(i = 0; i < 2; i ++)
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

	zBuffer = serialiseMPZ_Array(z_ToSend, 2, &zLength);
	cBuffer = serialiseMPZ_Array(cShares, 2, &cLength);
	aBuffer = (unsigned char*) calloc(sizeof(int) + (mpz_size(alphaAndA -> a) * sizeof(mp_limb_t)), sizeof(unsigned char));
	serialiseMPZ(alphaAndA -> a, aBuffer, &aLength);

	commBuffer = (unsigned char *) calloc(zLength + cLength + aLength, sizeof(unsigned char));

	memcpy(commBuffer, zBuffer, zLength);
	memcpy(commBuffer + zLength, cBuffer, cLength);
	memcpy(commBuffer + zLength + cLength, aBuffer, aLength);

	*outputLen = zLength + cLength + aLength;


	return commBuffer;
}


unsigned char *proverMessageTwo_ECC_1Of2(struct eccParams *params, unsigned char *J_set,
								struct verifierCommitment_ECC *commitment_box,
								struct msgOneArrays_ECC *msgArray, struct witnessStruct *witnessesArray,
								struct alphaAndA_Struct *alphaAndA, int *outputLen)
{
	unsigned char *commBuffer;
	mpz_t *cShares, *tempPointer;
	int i, checkC_Correct = 0, bufferOffset = 0;
	int tempIndex = 0;



	checkC_Correct = checkC_prover_ECC_1Of2(params, commitment_box, alphaAndA -> alpha);
	if(0 != checkC_Correct)
	{
		return NULL;
	}

	cShares = (mpz_t *) calloc(2, sizeof(mpz_t));
	tempIndex = msgArray -> notI_Struct -> not_in_I_index[0] - 1;
	mpz_init(cShares[1 - tempIndex]);
	mpz_init_set(cShares[tempIndex], msgArray -> notI_Struct -> C_array[0]);
	mpz_add(cShares[1 - tempIndex], cShares[tempIndex], commitment_box -> c);


	commBuffer = computeAndSerialise_ECC_1Of2(params, J_set, msgArray, witnessesArray, cShares, alphaAndA, &bufferOffset);


	*outputLen = bufferOffset;
	
	return commBuffer;
}


int verifierChecks_ECC_1Of2(struct eccParams *params,
					struct eccPoint **g_0, struct eccPoint **g_1,
					struct eccPoint **h_0_List, struct eccPoint **h_1_List,
					mpz_t *Z_array, struct eccPoint **A_array, struct eccPoint **B_array,
					struct alphaAndA_Struct *alphaAndA, mpz_t *codewords, mpz_t c)
{
	struct eccPoint *A_check, *B_check, *alpha;
	struct eccPoint *topHalf, *bottomHalf;

	mpz_t cShares2Check, cShares2CheckAbs;
	int i, alphaCheck = 0, AB_check = 0, finalDecision = 0;



	A_check = (struct eccPoint*) calloc(1, sizeof(struct eccPoint));
	B_check = (struct eccPoint*) calloc(1, sizeof(struct eccPoint));


	// alpha = windowedScalarPoint(alphaAndA -> a, params -> g, params);
	alpha = fixedPointMultiplication(gPreComputes, alphaAndA -> a, params);
	alphaCheck = eccPointsEqual(alpha, alphaAndA -> alpha);


	mpz_init(cShares2Check);
	mpz_init(cShares2CheckAbs);

	mpz_sub(cShares2Check, codewords[0], codewords[1]);
	mpz_abs(cShares2CheckAbs, cShares2Check);

	finalDecision = mpz_cmp(c, cShares2CheckAbs);


	for(i = 0; i < 2; i ++)
	{
		topHalf = windowedScalarPoint(Z_array[i], g_0[i], params);
		bottomHalf = windowedScalarPoint(codewords[i], h_0_List[i], params);
		A_check = invertPoint(bottomHalf, params);
		groupOp_PlusEqual(A_check, topHalf, params);
		
		clearECC_Point(topHalf);
		clearECC_Point(bottomHalf);

		topHalf = windowedScalarPoint(Z_array[i], g_1[i], params);
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



void ZKPoK_Prover_ECC_1Of2(int writeSocket, int readSocket,
					struct eccParams *params,
					struct eccPoint **g_0, struct eccPoint **g_1,
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


	witnessSet = proverSetupWitnesses_1(alphas_List);


	alphaAndA_P = proverSetupCommitment_ECC_1Of2(params, *state);


	bufferOffset = 0;
	commBuffer = (unsigned char *) calloc(sizeOfSerial_ECCPoint(alphaAndA_P -> alpha), sizeof(unsigned char));
	serialise_ECC_Point(alphaAndA_P -> alpha, commBuffer, &bufferOffset);
	sendBoth(writeSocket, commBuffer, bufferOffset);
	free(commBuffer);
	// Round 1


	bufferOffset = 0;
	commBufferLen = 0;
	commitment_box_P = initVerifierCommitment_ECC();


	commBuffer = receiveBoth(readSocket, commBufferLen);
	commitment_box_P -> C_commit = deserialise_ECC_Point(commBuffer, &bufferOffset);
	free(commBuffer);
	// Round 2

	bufferOffset = 0;
	msgOne_P = proverMessageOne_ECC_1Of2(params, J_set, alphaAndA_P -> alpha, g_0, g_1,
									h_0_List, h_1_List, *state);
	commBuffer = serialise_A_B_Arrays_ECC(msgOne_P, 2, &bufferOffset);
	sendBoth(writeSocket, commBuffer, bufferOffset);
	free(commBuffer);
	// Round 3

	bufferOffset = 0;
	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	deserialisedSecrets_CommitmentBox(commitment_box_P, commBuffer, &bufferOffset);
	free(commBuffer);
	// Round 4

	bufferOffset = 0;
	commBuffer = proverMessageTwo_ECC_1Of2(params, J_set, commitment_box_P, msgOne_P, witnessSet, alphaAndA_P, &bufferOffset);
	sendBoth(writeSocket, commBuffer, bufferOffset);
	free(commBuffer);
	// Round 5
}



int ZKPoK_Verifier_ECC_1Of2(int writeSocket, int readSocket, struct eccParams *params,
						struct eccPoint **g_0, struct eccPoint **g_1,
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


	commitment_box_V = verifierSetupCommitment_ECC_1Of2(params, alphaAndA_V -> alpha, *state);

	bufferOffset = 0;
	commBuffer = (unsigned char *) calloc(sizeOfSerial_ECCPoint(commitment_box_V -> C_commit), sizeof(unsigned char));
	serialise_ECC_Point(commitment_box_V -> C_commit, commBuffer, &bufferOffset);
	sendBoth(writeSocket, commBuffer, bufferOffset);
	free(commBuffer);
	// Round 2


	msgOne_V = initMsgOneArray_ECC(2);
	bufferOffset = 0;
	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);


	for(i = 0; i < 2; i ++)
	{
		msgOne_V -> A_array[i] = deserialise_ECC_Point(commBuffer, &bufferOffset);
		msgOne_V -> B_array[i] = deserialise_ECC_Point(commBuffer, &bufferOffset);
	}
	free(commBuffer);
	// Round 3


	bufferOffset = 0;
	commBuffer = verifierQuery_ECC_1Of2(commitment_box_V, &bufferOffset);
	sendBoth(writeSocket, commBuffer, bufferOffset);
	// Round 4
	free(commBuffer);

	bufferOffset = 0;
	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);


	Z_array_V = deserialiseMPZ_Array(commBuffer, &bufferOffset);
	cShares_V = deserialiseMPZ_Array(commBuffer, &bufferOffset);
	tempMPZ = deserialiseMPZ(commBuffer, &bufferOffset);
	mpz_set(alphaAndA_V -> a, *tempMPZ);
	free(commBuffer);

	verified = verifierChecks_ECC_1Of2(params, g_0, g_1, h_0_List, h_1_List, Z_array_V,
								msgOne_V -> A_array, msgOne_V -> B_array, alphaAndA_V, cShares_V, commitment_box_V -> c);


	for(i = 0; i < 2; i ++)
	{
		mpz_clear(Z_array_V[i]);
		mpz_clear(cShares_V[i]);
	}

	mpz_clear(*tempMPZ);
	free(Z_array_V);
	free(cShares_V);
	free(tempMPZ);

	return verified;
}






void test_ZKPoK_ECC_1Of2()
{
	struct params_CnC_ECC *params_P,  *params_V;
	struct witnessStruct *witnessSet;
	struct alphaAndA_Struct *alphaAndA_P, *alphaAndA_V;
	mpz_t *tempMPZ = (mpz_t*) calloc(1, sizeof(mpz_t));
	struct verifierCommitment_ECC *commitment_box_P, *commitment_box_V;
	struct msgOneArrays_ECC *msgOne_P, *msgOne_V;

	unsigned char *commBuffer;
	unsigned char sigmaBit = 0x00, *J_set;

	int i, j, k, numTests = 2, comp_SecParam = 1024, cCheck = 0, verified = 0;
	int bufferOffset = 0, u_v_index = 0, tempInt = 0;

	gmp_randstate_t *state = seedRandGen();


	J_set = generateJ_Set(numTests);
	params_P = setup_CnC_OT_Receiver_ECC(numTests, J_set, comp_SecParam, *state);
	commBuffer = serialiseParams_CnC_ECC(params_P, &bufferOffset);
	params_V = setup_CnC_OT_Sender_ECC(commBuffer);
	free(commBuffer);


	struct eccPoint **g_0 = (struct eccPoint **) calloc(2, sizeof(struct eccPoint*));
	struct eccPoint **g_1 = (struct eccPoint **) calloc(2, sizeof(struct eccPoint*));
	struct eccPoint **h_0 = (struct eccPoint **) calloc(2, sizeof(struct eccPoint*));
	struct eccPoint **h_1 = (struct eccPoint **) calloc(2, sizeof(struct eccPoint*));
	struct eccPoint **h_1Prime = (struct eccPoint **) calloc(2, sizeof(struct eccPoint*));



	mpz_init(*tempMPZ);

	mpz_urandomm(*tempMPZ, *state, params_P -> params -> n);
	g_0[0] = windowedScalarPoint(*tempMPZ, params_P -> params -> g, params_P -> params);
	g_0[1] = windowedScalarPoint(*tempMPZ, params_P -> params -> g, params_P -> params);

	mpz_urandomm(*tempMPZ, *state, params_P -> params -> n);
	g_1[0] = windowedScalarPoint(*tempMPZ, params_P -> params -> g, params_P -> params);
	g_1[1] = windowedScalarPoint(*tempMPZ, params_P -> params -> g, params_P -> params);

	mpz_urandomm(*tempMPZ, *state, params_P -> params -> n);
	h_0[0] = windowedScalarPoint(*tempMPZ, g_0[0], params_P -> params);
	// mpz_urandomm(*tempMPZ, *state, params_P -> params -> n);
	h_1[0] = windowedScalarPoint(*tempMPZ, g_1[0], params_P -> params);
	mpz_set(params_P -> crs -> alphas_List[0], *tempMPZ);

	mpz_urandomm(*tempMPZ, *state, params_P -> params -> n);
	h_0[1] = windowedScalarPoint(*tempMPZ, g_0[1], params_P -> params);
	mpz_urandomm(*tempMPZ, *state, params_P -> params -> n);
	h_1[1] = windowedScalarPoint(*tempMPZ, g_1[1], params_P -> params);
	mpz_set(params_P -> crs -> alphas_List[1], *tempMPZ);

	params_P -> crs -> J_set[0] = 0x01;
	params_P -> crs -> J_set[1] = 0x00;

	mpz_set(params_P -> crs -> alphas_List[1], params_P -> crs -> alphas_List[0]);
	// h_1Prime[0] = copyECC_Point(h_1[0]);
	// h_1Prime[1] = copyECC_Point(h_1[1]);
	// h_1[1] = copyECC_Point(h_1[1]);


	witnessSet = proverSetupWitnesses_1Of2(params_P -> crs -> alphas_List, params_P -> crs -> J_set);
	alphaAndA_P = proverSetupCommitment_ECC_1Of2(params_P -> params, *state);


	bufferOffset = 0;
	commBuffer = (unsigned char *) calloc(sizeOfSerial_ECCPoint(alphaAndA_P -> alpha), sizeof(unsigned char));
	serialise_ECC_Point(alphaAndA_P -> alpha, commBuffer, &bufferOffset);


	bufferOffset = 0;
	alphaAndA_V = (struct alphaAndA_Struct*) calloc(1, sizeof(struct alphaAndA_Struct));
	alphaAndA_V -> alpha = deserialise_ECC_Point(commBuffer, &bufferOffset);
	free(commBuffer);


	commitment_box_V = verifierSetupCommitment_ECC_1Of2(params_V -> params, alphaAndA_V -> alpha, *state);

	bufferOffset = 0;
	commBuffer = (unsigned char *) calloc(sizeOfSerial_ECCPoint(commitment_box_V -> C_commit), sizeof(unsigned char));
	serialise_ECC_Point(commitment_box_V -> C_commit, commBuffer, &bufferOffset);


	commitment_box_P = initVerifierCommitment_ECC();
	bufferOffset = 0;
	commitment_box_P -> C_commit = deserialise_ECC_Point(commBuffer, &bufferOffset);
	free(commBuffer);


	bufferOffset = 0;
	msgOne_P = proverMessageOne_ECC_1Of2(params_P -> params, params_P -> crs -> J_set, alphaAndA_P -> alpha, g_0, g_1, h_0, h_1, *state);
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
	commBuffer = verifierQuery_ECC_1Of2(commitment_box_V, &bufferOffset);


	bufferOffset = 0;
	deserialisedSecrets_CommitmentBox(commitment_box_P, commBuffer, &bufferOffset);
	free(commBuffer);


	bufferOffset = 0;
	commBuffer = proverMessageTwo_ECC_1Of2(params_P -> params, params_P -> crs -> J_set,
									commitment_box_P, msgOne_P, witnessSet, alphaAndA_P, &bufferOffset);


	bufferOffset = 0;
	mpz_t *Z_array_V = deserialiseMPZ_Array(commBuffer, &bufferOffset);
	mpz_t *cShares_V = deserialiseMPZ_Array(commBuffer, &bufferOffset);
	tempMPZ = deserialiseMPZ(commBuffer, &bufferOffset);
	mpz_set(alphaAndA_V -> a, *tempMPZ);


	verified = verifierChecks_ECC_1Of2(params_V -> params, g_0, g_1, h_0, h_1,
								Z_array_V, msgOne_V -> A_array, msgOne_V -> B_array,
								alphaAndA_V, cShares_V, commitment_box_V -> c);

	printf("Verified = %d\n", verified);
}


