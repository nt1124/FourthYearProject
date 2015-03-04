struct witnessStruct *proverSetupWitnesses(struct params_CnC *params)
{
	struct witnessStruct *witnessSet = initWitnessStruc(params -> crs -> stat_SecParam);
	int i, j = 0;


	for(i = 0; i < params -> crs -> stat_SecParam; i ++)
	{
		mpz_set(witnessSet -> witnesses[i], params -> crs -> alphas_List[i]);
		j++;
	}

	return witnessSet;
}


mpz_t *proverSetupCommitment(struct params_CnC *params, struct witnessStruct *witnessSet, gmp_randstate_t state)
{
	mpz_t *alphaAndA = (mpz_t*) calloc(2, sizeof(mpz_t));


	mpz_init(alphaAndA[0]);
	mpz_init(alphaAndA[1]);

	mpz_urandomm(alphaAndA[1], state, params -> group -> q);
	mpz_powm(alphaAndA[0], params -> group -> g, alphaAndA[1], params -> group -> p);


	return alphaAndA;
}


struct verifierCommitment *verifierSetupCommitment(struct params_CnC *params, mpz_t alpha, gmp_randstate_t state)
{
	struct verifierCommitment *commitment_box = initVerifierCommitment();
	mpz_t C_unmodded, temp1, temp2;


	mpz_init(temp1);
	mpz_init(temp2);
	mpz_init(C_unmodded);

	mpz_urandomm(commitment_box -> t, state, params -> group -> q);
	mpz_urandomm(commitment_box -> c, state, params -> group -> q);

	mpz_powm(temp1, params -> group -> g, commitment_box -> c, params -> group -> p);
	mpz_powm(temp2, alpha, commitment_box -> t, params -> group -> p);

	mpz_mul(C_unmodded, temp1, temp2);
	mpz_mod(commitment_box -> C_commit, C_unmodded, params -> group -> p);


	return commitment_box;
}



struct msgOneArrays *proverMessageOne(struct params_CnC *params, mpz_t alpha, gmp_randstate_t state)
{
	struct msgOneArrays *msgArray = initMsgOneArray(params -> crs -> stat_SecParam);
	mpz_t topHalf, bottomHalf, bottomHalf_inv, unmodded, temp;
	int i, j_in_I = 0, j_not_I = 0;


	mpz_init(topHalf);
	mpz_init(bottomHalf);
	mpz_init(bottomHalf_inv);
	mpz_init(unmodded);
	mpz_init(temp);


	for(i = 0; i < params -> crs -> stat_SecParam; i ++)
	{
		// If i IS in I
		if(0x00 == params -> crs -> J_set[i])
		{
			msgArray -> notI_Struct -> not_in_I_index[j_not_I] = i + 1;

			mpz_urandomm(msgArray -> notI_Struct -> C_array[j_not_I], state, params -> group -> q);
			mpz_urandomm(msgArray -> notI_Struct -> Z_array[j_not_I], state, params -> group -> q);
			
			mpz_powm(topHalf, params -> group -> g, msgArray -> notI_Struct -> Z_array[j_not_I], params -> group -> p);
			mpz_powm(bottomHalf, params -> crs -> h_0_List[i], msgArray -> notI_Struct -> C_array[j_not_I], params -> group -> p);
			mpz_invert(bottomHalf_inv, bottomHalf, params -> group -> p);
			mpz_mul(unmodded, topHalf, bottomHalf_inv);
			mpz_mod(msgArray -> A_array[i], unmodded, params -> group -> p);

			mpz_powm(topHalf, params -> crs -> g_1, msgArray -> notI_Struct -> Z_array[j_not_I], params -> group -> p);
			mpz_powm(bottomHalf, params -> crs -> h_1_List[i], msgArray -> notI_Struct -> C_array[j_not_I], params -> group -> p);
			mpz_invert(bottomHalf_inv, bottomHalf, params -> group -> p);
			mpz_mul(unmodded, topHalf, bottomHalf_inv);
			mpz_mod(msgArray -> B_array[i], unmodded, params -> group -> p);

			j_not_I ++;
		}
		else
		{
			msgArray -> in_I_index[j_in_I] = i + 1;

			mpz_urandomm(msgArray -> roeArray[j_in_I], state, params -> group -> q);

			mpz_powm(msgArray -> A_array[i], params -> group -> g, msgArray -> roeArray[j_in_I], params -> group -> p);
			mpz_powm(msgArray -> B_array[i], params -> crs -> g_1, msgArray -> roeArray[j_in_I], params -> group -> p);

			j_in_I ++;
		}
	}


	return msgArray;
}


unsigned char *verifierQuery(struct verifierCommitment *commitment_box, int *outputLen)
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


int checkC_prover(struct params_CnC *params, struct verifierCommitment *commitment_box, mpz_t alpha)
{
	mpz_t checkC, checkC_unmodded, temp1, temp2;
	int i, checkC_Correct = 0;


	mpz_init(temp1);
	mpz_init(temp2);
	mpz_init(checkC);
	mpz_init(checkC_unmodded);

	// Check the C = (g_0)^c * (alpha)^t
	mpz_powm(temp1, params -> group -> g, commitment_box -> c, params -> group -> p);
	mpz_powm(temp2, alpha, commitment_box -> t, params -> group -> p);
	mpz_mul(checkC_unmodded, temp1, temp2);
	mpz_mod(checkC, checkC_unmodded, params -> group -> p);

	checkC_Correct = mpz_cmp(checkC, commitment_box -> C_commit);


	return checkC_Correct;
}


unsigned char *computeAndSerialise(struct params_CnC *params, struct msgOneArrays *msgArray, struct witnessStruct *witnessesArray,
									mpz_t *cShares, mpz_t *alphaAndA, int *outputLen)
{
	unsigned char *commBuffer, *zBuffer, *cBuffer, *aBuffer;
	mpz_t temp1, temp2, *z_ToSend, *temp;
	int i, j_in_I = 0, j_not_I = 0;
	int bufferOffset = sizeof(int), totalLength;
	int zLength = 0, cLength = 0, aLength = 0;


	mpz_init(temp1);
	mpz_init(temp2);

	totalLength = (2 * params -> crs -> stat_SecParam + 3) * sizeof(int);
	z_ToSend = (mpz_t*) calloc(params -> crs -> stat_SecParam, sizeof(mpz_t));

	for(i = 0; i < params -> crs -> stat_SecParam; i ++)
	{
		mpz_init(z_ToSend[i]);

		// i IS in I = J^{bar}
		if(0x00 == params -> crs -> J_set[i])
		{
			mpz_set(z_ToSend[i], msgArray -> notI_Struct -> Z_array[j_not_I]);

			j_not_I ++;
		}
		else
		{
			// z_i = c_i * w_i + ρ_i
			mpz_mul(temp1, cShares[i], witnessesArray -> witnesses[i]);
			mpz_add(temp2, temp1, msgArray -> roeArray[j_in_I]);
			mpz_mod(z_ToSend[i], temp2, params -> group -> q);


			j_in_I ++;
		}
	}

	zBuffer = serialiseMPZ_Array(z_ToSend, params -> crs -> stat_SecParam, &zLength);
	cBuffer = serialiseMPZ_Array(cShares, params -> crs -> stat_SecParam, &cLength);
	aBuffer = (unsigned char*) calloc(sizeof(int) + (mpz_size(alphaAndA[1]) * sizeof(mp_limb_t)), sizeof(unsigned char));
	serialiseMPZ(alphaAndA[1], aBuffer, &aLength);

	commBuffer = (unsigned char *) calloc(zLength + cLength + aLength, sizeof(unsigned char));

	memcpy(commBuffer, zBuffer, zLength);
	memcpy(commBuffer + zLength, cBuffer, cLength);
	memcpy(commBuffer + zLength + cLength, aBuffer, aLength);

	*outputLen = zLength + cLength;


	// temp = deserialiseMPZ(commBuffer, outputLen);

	return commBuffer;
}



unsigned char *proverMessageTwo(struct params_CnC *params, struct verifierCommitment *commitment_box, struct msgOneArrays *msgArray,
								struct witnessStruct *witnessesArray, mpz_t *alphaAndA, int *outputLen)
{
	unsigned char *commBuffer;
	mpz_t temp1, *cShares, *tempPointer;
	int i, checkC_Correct = 0, totalLength, bufferOffset = 0;
	int *tempDeltaI = (int*) calloc(params -> crs -> stat_SecParam, sizeof(int));


	checkC_Correct = checkC_prover(params, commitment_box, alphaAndA[0]);
	if(0 != checkC_Correct)
	{
		return NULL;
	}

	for(i = 0; i < params -> crs -> stat_SecParam; i ++)
	{
		tempDeltaI[i] = i + 1;
	}

	// Here the secret sharing distrbution happens. Reed-Solomon etc.
	cShares = completePartialSecretSharing(msgArray -> notI_Struct -> not_in_I_index, msgArray -> notI_Struct -> C_array,
											commitment_box -> c, params -> group -> q, params -> crs -> stat_SecParam);

	struct Fq_poly *tempPoly = getPolyFromCodewords(cShares, tempDeltaI, params -> crs -> stat_SecParam, params -> group -> p);

	mpz_init_set_ui(temp1, (params -> crs -> stat_SecParam + 1));
	tempPointer = evalutePoly(tempPoly, temp1, params -> group -> p);


	commBuffer = computeAndSerialise(params, msgArray, witnessesArray, cShares, alphaAndA, &bufferOffset);


	*outputLen = bufferOffset;
	
	return commBuffer;
}


int verifierChecks(struct params_CnC *params, mpz_t *Z_array, mpz_t *A_array, mpz_t *B_array,
					mpz_t *alphaAndA, mpz_t *codewords, mpz_t c)
{
	struct Fq_poly *secretPoly;
	mpz_t alpha, *A_check_array, *B_check_array;
	mpz_t topHalf, bottomHalf, bottomHalf_inv, unmodded;
	int *delta_i = (int*) calloc(params -> crs -> stat_SecParam, sizeof(int*));
	int i, alphaCheck = 0, AB_check = 0, finalDecision = 0;


	mpz_init(alpha);
	mpz_init(topHalf);
	mpz_init(bottomHalf);
	mpz_init(bottomHalf_inv);
	mpz_init(unmodded);

	A_check_array = (mpz_t*) calloc(params -> crs -> stat_SecParam, sizeof(mpz_t));
	B_check_array = (mpz_t*) calloc(params -> crs -> stat_SecParam, sizeof(mpz_t));

	mpz_powm(alpha, params -> group -> g, alphaAndA[1], params -> group -> p);
	alphaCheck = mpz_cmp(alpha, alphaAndA[0]);


	for(i = 0; i < params -> crs -> stat_SecParam; i ++)
	{
		delta_i[i] = i + 1;
	}

	secretPoly = getPolyFromCodewords(codewords, delta_i, params -> crs -> stat_SecParam / 2 + 1, params -> group -> q);
	finalDecision = testSecretScheme(secretPoly, c, params -> group -> q, (params -> crs -> stat_SecParam / 2) + 1, params -> crs -> stat_SecParam);
	// finalDecision = testSecretScheme(secretPoly, c, params -> group -> q, params -> crs -> stat_SecParam / 2, params -> crs -> stat_SecParam);

	for(i = 0; i < params -> crs -> stat_SecParam; i ++)
	{
		mpz_init(A_check_array[i]);
		mpz_init(B_check_array[i]);

		mpz_powm(topHalf, params -> group -> g, Z_array[i], params -> group -> p);
		mpz_powm(bottomHalf, params -> crs -> h_0_List[i], codewords[i], params -> group -> p);
		mpz_invert(bottomHalf_inv, bottomHalf, params -> group -> p);
		mpz_mul(unmodded, topHalf, bottomHalf_inv);
		mpz_mod(A_check_array[i], unmodded, params -> group -> p);

		mpz_powm(topHalf, params -> crs -> g_1, Z_array[i], params -> group -> p);
		mpz_powm(bottomHalf, params -> crs -> h_1_List[i], codewords[i], params -> group -> p);
		mpz_invert(bottomHalf_inv, bottomHalf, params -> group -> p);
		mpz_mul(unmodded, topHalf, bottomHalf_inv);
		mpz_mod(B_check_array[i], unmodded, params -> group -> p);


		AB_check |= mpz_cmp(A_array[i], A_check_array[i]);
		AB_check |= mpz_cmp(B_array[i], B_check_array[i]);
	}


	// printf("%d  -  %d  -  %d\n", alphaCheck, AB_check, finalDecision);
	finalDecision |= AB_check;
	finalDecision |= alphaCheck;

	return finalDecision;
}



void ZKPoK_Prover(int writeSocket, int readSocket, struct params_CnC *params_P, gmp_randstate_t *state)
{
	struct witnessStruct *witnessSet;
	mpz_t *alphaAndA_P, *tempMPZ;
	struct verifierCommitment *commitment_box_P;
	struct msgOneArrays *msgOne_P;

	unsigned char *commBuffer;
	int bufferOffset = 0, commBufferLen = 0;


	witnessSet = proverSetupWitnesses(params_P);
	alphaAndA_P = proverSetupCommitment(params_P, witnessSet, *state);

	bufferOffset = 0;
	commBuffer = (unsigned char *) calloc(sizeof(int) + (sizeof(mp_limb_t) * mpz_size(alphaAndA_P[0])), sizeof(unsigned char));
	serialiseMPZ(alphaAndA_P[0], commBuffer, &bufferOffset);
	sendBoth(writeSocket, commBuffer, bufferOffset);
	// Round 1

	bufferOffset = 0;
	commBufferLen = 0;
	commitment_box_P = initVerifierCommitment();

	commBuffer = receiveBoth(readSocket, commBufferLen);
	tempMPZ = deserialiseMPZ(commBuffer, &bufferOffset);
	mpz_init_set(commitment_box_P -> C_commit, *tempMPZ);
	free(commBuffer);
	// Round 2

	bufferOffset = 0;
	msgOne_P = proverMessageOne(params_P, alphaAndA_P[0], *state);
	commBuffer = serialise_A_B_Arrays(msgOne_P, params_P -> crs -> stat_SecParam, &bufferOffset);
	sendBoth(writeSocket, commBuffer, bufferOffset);
	// Round 3

	bufferOffset = 0;
	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	deserialisedSecrets_CommitmentBox(commitment_box_P, commBuffer, &bufferOffset);
	free(commBuffer);
	// Round 4


	bufferOffset = 0;
	commBuffer = proverMessageTwo(params_P, commitment_box_P, msgOne_P, witnessSet, alphaAndA_P, &bufferOffset);
	sendBoth(writeSocket, commBuffer, bufferOffset);
	// Round 5
}


int ZKPoK_Verifier(int writeSocket, int readSocket, struct params_CnC *params_V, gmp_randstate_t *state)
{
	mpz_t *alphaAndA_V = (mpz_t*) calloc(2, sizeof(mpz_t)), *tempMPZ;
	struct verifierCommitment *commitment_box_V;
	struct msgOneArrays *msgOne_V;
	unsigned char *commBuffer;

	int cCheck = 0, verified = 0;
	int bufferOffset = 0, commBufferLen = 0;


	bufferOffset = 0;
	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	tempMPZ = deserialiseMPZ(commBuffer, &bufferOffset);
	mpz_init_set(alphaAndA_V[0], *tempMPZ);
	free(commBuffer);
	// Round 1

	commitment_box_V = verifierSetupCommitment(params_V, alphaAndA_V[0], *state);

	bufferOffset = 0;
	commBuffer = (unsigned char *) calloc(sizeof(int) + (sizeof(mp_limb_t) * mpz_size(commitment_box_V -> C_commit)), sizeof(unsigned char));
	serialiseMPZ(commitment_box_V -> C_commit, commBuffer, &bufferOffset);
	sendBoth(writeSocket, commBuffer, bufferOffset);
	// Round 2


	msgOne_V = initMsgOneArray(params_V -> crs -> stat_SecParam);
	bufferOffset = 0;
	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	msgOne_V -> A_array = deserialiseMPZ_Array(commBuffer, &bufferOffset);
	msgOne_V -> B_array = deserialiseMPZ_Array(commBuffer, &bufferOffset);
	free(commBuffer);
	// Round 3

	bufferOffset = 0;
	commBuffer = verifierQuery(commitment_box_V, &bufferOffset);
	sendBoth(writeSocket, commBuffer, bufferOffset);
	// Round 4

	bufferOffset = 0;
	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	mpz_t *Z_array_V = deserialiseMPZ_Array(commBuffer, &bufferOffset);
	mpz_t *cShares_V = deserialiseMPZ_Array(commBuffer, &bufferOffset);
	tempMPZ = deserialiseMPZ(commBuffer, &bufferOffset);
	mpz_set(alphaAndA_V[1], *tempMPZ);

	

	verified = verifierChecks(params_V, Z_array_V, msgOne_V -> A_array, msgOne_V -> B_array, alphaAndA_V, cShares_V, commitment_box_V -> c);

	return verified;
}



int test_ZKPoK()
{
	struct params_CnC *params_P,  *params_V;
	struct witnessStruct *witnessSet;
	mpz_t *alphaAndA_P, *alphaAndA_V, *tempMPZ;
	struct verifierCommitment *commitment_box_P, *commitment_box_V;
	struct msgOneArrays *msgOne_P, *msgOne_V;

	unsigned char *commBuffer;
	unsigned char sigmaBit = 0x00;

	int i, j, k, numTests = 10, comp_SecParam = 1024, cCheck = 0, verified = 0;
	int bufferOffset = 0, u_v_index = 0, tempInt = 0;

	gmp_randstate_t *state = seedRandGen();


	params_P = setup_CnC_OT_Receiver(numTests, comp_SecParam, *state);
	commBuffer = serialiseParams_CnC(params_P, &bufferOffset);
	params_V = setup_CnC_OT_Sender(commBuffer);
	free(commBuffer);


	witnessSet = proverSetupWitnesses(params_P);
	alphaAndA_P = proverSetupCommitment(params_P, witnessSet, *state);


	bufferOffset = 0;
	commBuffer = (unsigned char *) calloc(sizeof(int) + (sizeof(mp_limb_t) * mpz_size(alphaAndA_P[0])), sizeof(unsigned char));
	serialiseMPZ(alphaAndA_P[0], commBuffer, &bufferOffset);


	alphaAndA_V = (mpz_t*) calloc(2, sizeof(mpz_t));
	bufferOffset = 0;
	tempMPZ = deserialiseMPZ(commBuffer, &bufferOffset);
	mpz_init_set(alphaAndA_V[0], *tempMPZ);
	free(commBuffer);


	commitment_box_V = verifierSetupCommitment(params_V, alphaAndA_V[0], *state);

	
	bufferOffset = 0;
	commBuffer = (unsigned char *) calloc(sizeof(int) + (sizeof(mp_limb_t) * mpz_size(commitment_box_V -> C_commit)), sizeof(unsigned char));
	serialiseMPZ(commitment_box_V -> C_commit, commBuffer, &bufferOffset);

	
	commitment_box_P = initVerifierCommitment();
	bufferOffset = 0;
	tempMPZ = deserialiseMPZ(commBuffer, &bufferOffset);
	mpz_init_set(commitment_box_P -> C_commit, *tempMPZ);
	free(commBuffer);


	bufferOffset = 0;
	msgOne_P = proverMessageOne(params_P, alphaAndA_P[0], *state);
	commBuffer = serialise_A_B_Arrays(msgOne_P, params_P -> crs -> stat_SecParam, &bufferOffset);

	msgOne_V = initMsgOneArray(params_V -> crs -> stat_SecParam);
	bufferOffset = 0;
	msgOne_V -> A_array = deserialiseMPZ_Array(commBuffer, &bufferOffset);
	msgOne_V -> B_array = deserialiseMPZ_Array(commBuffer, &bufferOffset);
	free(commBuffer);


	bufferOffset = 0;
	commBuffer = verifierQuery(commitment_box_V, &bufferOffset);

	bufferOffset = 0;
	deserialisedSecrets_CommitmentBox(commitment_box_P, commBuffer, &bufferOffset);
	free(commBuffer);

	bufferOffset = 0;
	commBuffer = proverMessageTwo(params_P, commitment_box_P, msgOne_P, witnessSet, alphaAndA_P, &bufferOffset);


	bufferOffset = 0;
	mpz_t *Z_array_V = deserialiseMPZ_Array(commBuffer, &bufferOffset);
	mpz_t *cShares_V = deserialiseMPZ_Array(commBuffer, &bufferOffset);
	tempMPZ = deserialiseMPZ(commBuffer, &bufferOffset);
	mpz_set(alphaAndA_V[1], *tempMPZ);

	
	verified = verifierChecks(params_V, Z_array_V, msgOne_V -> A_array, msgOne_V -> B_array, alphaAndA_V, cShares_V, commitment_box_V -> c);

	printf("%d\n",verified);

}