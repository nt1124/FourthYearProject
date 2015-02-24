struct witnessStruct *initWitnessStruc(int length)
{
	struct witnessStruct *toReturn = (struct witnessStruct *) calloc(1, sizeof(struct witnessStruct));
	int i;


	toReturn -> IDs = (int*) calloc(length, sizeof(int));
	toReturn -> witnesses = (mpz_t*) calloc(length, sizeof(mpz_t));

	for(i = 0; i < length; i ++)
	{
		mpz_init(toReturn -> witnesses[i]);
	}


	return toReturn;
}


struct verifierCommitment *initVerifierCommitment()
{
	struct verifierCommitment *commitment_box = (struct verifierCommitment *) calloc(1, sizeof(struct verifierCommitment));

	mpz_init(commitment_box -> t);
	mpz_init(commitment_box -> c);
	mpz_init(commitment_box -> C_commit);

	return commitment_box;
}


struct msgOne_notI_Arrays *initMsgOne_notI_Array(int stat_SecParam)
{
	struct msgOne_notI_Arrays *msg_I_One = (struct msgOne_notI_Arrays *) calloc(1, sizeof(struct msgOne_notI_Arrays));
	int i, stat_SecParamDiv2 = stat_SecParam / 2;


	msg_I_One -> not_in_I_index = (int *) calloc(stat_SecParamDiv2, sizeof(int));
	msg_I_One -> Z_array = (mpz_t *) calloc(stat_SecParamDiv2, sizeof(mpz_t));
	msg_I_One -> C_array = (mpz_t *) calloc(stat_SecParamDiv2, sizeof(mpz_t));

	for(i = 0; i < stat_SecParamDiv2; i ++)
	{
		mpz_init(msg_I_One -> Z_array[i]);
		mpz_init(msg_I_One -> C_array[i]);
	}

	return msg_I_One;
}


struct msgOneArrays *initMsgOneArray(int stat_SecParam)
{
	struct msgOneArrays *msgOne = (struct msgOneArrays *) calloc(1, sizeof(struct msgOneArrays));
	int i, stat_SecParamDiv2 = stat_SecParam / 2;


	msgOne -> in_I_index = (int *) calloc(stat_SecParamDiv2, sizeof(int));
	msgOne -> roeArray = (mpz_t *) calloc(stat_SecParamDiv2, sizeof(mpz_t));
	msgOne -> A_array = (mpz_t*) calloc(stat_SecParam, sizeof(mpz_t));
	msgOne -> B_array = (mpz_t*) calloc(stat_SecParam, sizeof(mpz_t));

	msgOne -> notI_Struct = initMsgOne_notI_Array(stat_SecParam);

	for(i = 0; i < stat_SecParamDiv2; i ++)
	{
		mpz_init(msgOne -> roeArray[i]);
		mpz_init(msgOne -> A_array[i]);
		mpz_init(msgOne -> B_array[i]);
	}
	for(i = stat_SecParamDiv2; i < stat_SecParam; i ++)
	{
		mpz_init(msgOne -> A_array[i]);
		mpz_init(msgOne -> B_array[i]);
	}


	return msgOne;
}


struct witnessStruct *proverSetupWitnesses(struct params_CnC *params)
{
	struct witnessStruct *witnessSet = initWitnessStruc(params -> crs -> stat_SecParam / 2);
	int i, j = 0;


	for(i = 0; i < params -> crs -> stat_SecParam / 2; i ++)
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

	mpz_urandomm(alphaAndA[1], state, params -> group -> p);
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

	mpz_urandomm(commitment_box -> t, state, params -> group -> p);
	mpz_urandomm(commitment_box -> c, state, params -> group -> p);

	mpz_powm(temp1, params -> group -> g, commitment_box -> c, params -> group -> p);
	mpz_powm(temp2, alpha, commitment_box -> t, params -> group -> p);

	mpz_mul(C_unmodded, temp1, temp2);
	mpz_mod(commitment_box -> C_commit, C_unmodded, params -> group -> p);


	return commitment_box;
}



struct msgOneArrays *proverMessageOne(struct params_CnC *params, mpz_t alpha, gmp_randstate_t state)
{
	struct msgOneArrays *msgArray = initMsgOneArray(params -> crs -> stat_SecParam);
	mpz_t denom, numer, numer_inv, unmodded, temp;
	int i, j_in_I = 0, j_not_I = 0;


	mpz_init(denom);
	mpz_init(numer);
	mpz_init(numer_inv);
	mpz_init(unmodded);
	mpz_init(temp);


	for(i = 0; i < params -> crs -> stat_SecParam; i ++)
	{
		// If i IS in I
		if(0x00 == params -> crs -> J_set[i])
		{
			msgArray -> in_I_index[j_in_I] = i;
			mpz_urandomm(msgArray -> roeArray[j_in_I], state, params -> group -> q);

			mpz_powm(msgArray -> A_array[i], params -> group -> g, msgArray -> roeArray[j_in_I], params -> group -> p);
			mpz_powm(msgArray -> B_array[i], params -> crs -> g_1, msgArray -> roeArray[j_in_I], params -> group -> p);

			j_in_I ++;
		}
		else
		{
			msgArray -> notI_Struct -> not_in_I_index[j_not_I] = i;
			
			mpz_urandomm(temp, state, params -> group -> q);
			mpz_set(msgArray -> notI_Struct -> C_array[j_not_I], temp);
			
			mpz_urandomm(temp, state, params -> group -> q);
			mpz_set(msgArray -> notI_Struct -> Z_array[j_not_I], temp);
			

			mpz_powm(denom, params -> group -> g, msgArray -> notI_Struct -> C_array[j_not_I], params -> group -> p);
			mpz_powm(numer, params -> crs -> h_0_List[i], msgArray -> notI_Struct -> Z_array[j_not_I], params -> group -> p);
			mpz_invert(numer_inv, numer, params -> group -> p);
			mpz_mul(unmodded, denom, numer_inv);
			mpz_mod(msgArray -> A_array[i], unmodded, params -> group -> p);

			mpz_powm(denom, params -> crs -> g_1, msgArray -> notI_Struct -> C_array[j_not_I], params -> group -> p);
			mpz_powm(numer, params -> crs -> h_1_List[i], msgArray -> notI_Struct -> Z_array[j_not_I], params -> group -> p);
			mpz_invert(numer_inv, numer, params -> group -> p);
			mpz_mul(unmodded, denom, numer_inv);
			mpz_mod(msgArray -> B_array[i], unmodded, params -> group -> p);

			j_not_I ++;
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
									mpz_t *alphaAndA, int *outputLen)
{
	unsigned char *commBuffer;
	mpz_t temp1, temp2, *z_ToSend;
	int i, j_in_I = 0, j_not_I = 0;
	int bufferOffset = 0, totalLength;


	totalLength = (params -> crs -> stat_SecParam + 1) * sizeof(int);
	z_ToSend = (mpz_t*) calloc(params -> crs -> stat_SecParam, sizeof(mpz_t));

	for(i = 0; i < params -> crs -> stat_SecParam; i ++)
	{
		printf(":: %d  =  %X\n", i, params -> crs -> J_set[i]);
		fflush(stdout);
		mpz_init(z_ToSend[i]);

		// i IS in I = J^{bar}
		if(0x00 == params -> crs -> J_set[i])
		{
			// zi = c_i * w_i + ρ_i
			mpz_mul(temp1, msgArray -> notI_Struct -> C_array[j_not_I], witnessesArray -> witnesses[j_not_I]);
			mpz_add(temp2, temp1, msgArray -> notI_Struct -> Z_array[j_not_I]);
			mpz_mod(z_ToSend[i], temp2, params -> group -> q);

			j_not_I ++;
		}
		else
		{
			mpz_set(z_ToSend[i], msgArray -> notI_Struct -> Z_array[j_in_I]);

			j_in_I ++;
		}

		totalLength += ( sizeof(mp_limb_t) * mpz_size(z_ToSend[i]) );
	}


	printf("{}\n");
	fflush(stdout);

	totalLength += (sizeof(mp_limb_t) * mpz_size(alphaAndA[1]));

	printf("{}\n");
	fflush(stdout);

	commBuffer = (unsigned char *) calloc(totalLength, sizeof(unsigned char));
	for(i = 0; i < params -> crs -> stat_SecParam; i ++)
	{
		printf("%d\n", i);
		fflush(stdout);

		serialiseMPZ(z_ToSend[i], commBuffer, &bufferOffset);
	}
	/*
	serialiseMPZ(alphaAndA[1], commBuffer, &bufferOffset);
	*/

	*outputLen = bufferOffset;

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

	for(i = 0; i < params -> crs -> stat_SecParam - 1; i ++)
	{
		tempDeltaI[i] = i + 1;
	}

	// Here the secret sharing distrbution happens. Reed-Solomon etc.
	cShares = completePartialSecretSharing(msgArray -> notI_Struct -> not_in_I_index, msgArray -> notI_Struct -> C_array,
											commitment_box -> C_commit, params -> group -> p, params -> crs -> stat_SecParam);

	struct Fq_poly *tempPoly = getPolyFromCodewords(cShares, tempDeltaI, params -> crs -> stat_SecParam, params -> group -> p);

	mpz_init_set_ui(temp1, (params -> crs -> stat_SecParam + 1));
	tempPointer = evalutePoly(tempPoly, temp1, params -> group -> p);
	gmp_printf("<> %Zd\n... %Zd\n", *tempPointer, temp1);
	fflush(stdout);


	commBuffer = computeAndSerialise(params, msgArray, witnessesArray, alphaAndA, &bufferOffset);

	return commBuffer;
}


int verifierChecks(struct params_CnC *params, struct msgOneArrays *msgArray, mpz_t *alphaAndA)
{
	mpz_t alpha, *A_check_array, *B_check_array;
	mpz_t denom, numer, numer_inv, unmodded;
	int i, alphaCheck = 0;
	int finalDecision = 0;


	mpz_init(alpha);
	mpz_init(denom);
	mpz_init(numer);
	mpz_init(numer_inv);
	mpz_init(unmodded);

	A_check_array = (mpz_t*) calloc(params -> crs -> stat_SecParam, sizeof(mpz_t));
	B_check_array = (mpz_t*) calloc(params -> crs -> stat_SecParam, sizeof(mpz_t));

	mpz_powm(alpha, params -> group -> g, alphaAndA[1], params -> group -> p);
	alphaCheck = mpz_cmp(alpha, alphaAndA[0]);

	for(i = 0; i < params -> crs -> stat_SecParam; i ++)
	{
		mpz_init(A_check_array[i]);
		mpz_init(B_check_array[i]);

		/*
		mpz_powm(denom, params -> group -> g, msgArray -> in_I_Struct -> C_array[j_not_I], params -> group -> p);
		mpz_powm(numer, params -> crs -> h_0_List[i], msgArray -> in_I_Struct -> Z_array[j_not_I], params -> group -> p);
		mpz_invert(numer_inv, numer, params -> group -> p);
		mpz_mul(unmodded, denom, numer_inv);
		mpz_mod(A_check_array[i], unmodded, params -> group -> p);

		mpz_powm(denom, params -> crs -> g_1, msgArray -> in_I_Struct -> C_array[j_not_I], params -> group -> p);
		mpz_powm(numer, params -> crs -> h_1_List[i], msgArray -> in_I_Struct -> Z_array[j_not_I], params -> group -> p);
		mpz_invert(numer_inv, numer, params -> group -> p);
		mpz_mul(unmodded, denom, numer_inv);
		mpz_mod(B_check_array[i], unmodded, params -> group -> p);
		*/
	}


	return finalDecision;
}



int test_ZKPoK()
{
	struct params_CnC *params;
	struct witnessStruct *witnessSet;
	mpz_t *alphaAndA;
	struct verifierCommitment *commitment_box;
	struct msgOneArrays *msgOne;

	unsigned char *commBuffer;
	unsigned char sigmaBit = 0x00;

	int i, j, k, numTests = 128, comp_SecParam = 1024, cCheck = 0;
	int bufferOffset = 0, u_v_index = 0, tempInt = 0;

	gmp_randstate_t *state = seedRandGen();


	params = setup_CnC_OT_Receiver(numTests, comp_SecParam, *state);


	witnessSet = proverSetupWitnesses(params);
	alphaAndA = proverSetupCommitment(params, witnessSet, *state);
	commitment_box = verifierSetupCommitment(params, alphaAndA[0], *state);
	msgOne = proverMessageOne(params, alphaAndA[0], *state);
	commBuffer = verifierQuery(commitment_box, &bufferOffset);
	commBuffer = proverMessageTwo(params, commitment_box, msgOne, witnessSet, alphaAndA, &bufferOffset);


	/*

	unsigned char *computeAndSerialise(struct params_CnC *params, struct msgOneArrays *msgArray, struct witnessStruct *witnessesArray,
										mpz_t *alphaAndA, int *outputLen)

	unsigned char *proverMessageTwo(struct params_CnC *params, struct verifierCommitment *commitment_box, struct msgOneArrays *msgArray,
									struct witnessStruct *witnessesArray, mpz_t *alphaAndA, int *outputLen)


	verifierChecks(struct params_CnC *params, struct msgOneArrays *msgArray, mpz_t *alphaAndA)
	*/

}