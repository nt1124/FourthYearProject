
struct u_v_Pair_ECC *randomiseDDH_ECC(struct ECC_PK *pk, struct eccParams *params, gmp_randstate_t state)
{
	struct u_v_Pair_ECC *toReturn = (struct u_v_Pair_ECC*) calloc(1, sizeof(struct u_v_Pair_ECC));
	struct eccPoint *tempScalarS, *tempScalarT;
	mpz_t s, t;
	mpz_t tempPowS, tempPowT;
	mpz_t temp;

	// printf(">>>>>>.\n");

	mpz_init(s);
	mpz_init(t);

	mpz_urandomm(s, state, params -> n);
	mpz_urandomm(t, state, params -> n);

	tempScalarS = windowedScalarPoint(s, pk -> g, params);
	tempScalarT = windowedScalarPoint(t, pk -> h, params);
	toReturn -> u = groupOp(tempScalarS, tempScalarT, params);
	clearECC_Point(tempScalarS);
	clearECC_Point(tempScalarT);


	tempScalarS = windowedScalarPoint(s, pk -> g_x, params);
	tempScalarT = windowedScalarPoint(t, pk -> h_x, params);
	toReturn -> v = groupOp(tempScalarS, tempScalarT, params);
	clearECC_Point(tempScalarS);
	clearECC_Point(tempScalarT);

	mpz_clear(s);
	mpz_clear(t);

	return toReturn;
}


struct eccPoint *generate_ECC_KeyPair(struct eccParams *params, mpz_t SK, gmp_randstate_t state)
{
	struct eccPoint *PK = initECC_Point();


	do
	{
		mpz_urandomm(SK, state, params -> n);
	} while( 0 == mpz_cmp_ui(SK, 0) );

	PK = windowedScalarPoint(SK, params -> g, params);


	return PK;
}



struct u_v_Pair_ECC *ECC_Enc(struct ECC_PK *pk, struct eccPoint *msg, struct eccParams *params, gmp_randstate_t state)
{
	struct u_v_Pair_ECC *ciphertext;

	ciphertext = randomiseDDH_ECC(pk, params, state);
	groupOp_PlusEqual(ciphertext -> v, msg, params);

	return ciphertext;
}


struct eccPoint *ECC_Dec(mpz_t SK, struct u_v_Pair_ECC *ciphertext, struct eccParams *params)
{
	struct eccPoint *plaintext, *SK_Scalar_Key, *invKey;


	SK_Scalar_Key = windowedScalarPoint(SK, ciphertext -> u, params);

	invKey = invertPoint(SK_Scalar_Key, params);
	plaintext = groupOp(ciphertext -> v, invKey, params);

	clearECC_Point(SK_Scalar_Key);
	clearECC_Point(invKey);


	return plaintext;
}


struct eccPoint *ECC_Dec_Alt(mpz_t sk, mpz_t y, struct u_v_Pair_ECC *CT, struct eccParams *params, unsigned char sigmaBit)
{
	struct eccPoint *plaintext, *SK_Scalar_Key, *invKey, *invKeyY;
	mpz_t *M = (mpz_t *) calloc(1, sizeof(mpz_t));
	mpz_t c1_sk, c1_sk_inv;
	mpz_t M_unmodded;
	mpz_t finalFactor, y_Inv, M_y;

	mpz_init(*M);
	mpz_init(M_y);
	mpz_init(M_unmodded);
	mpz_init(c1_sk);
	mpz_init(c1_sk_inv);
	mpz_init(finalFactor);
	mpz_init(y_Inv);


	if(0x00 == sigmaBit)
	{
		mpz_invert(y_Inv, y, params -> n);

		mpz_mul(finalFactor, y_Inv, sk);
		mpz_mod(y_Inv, finalFactor, params -> n);

		SK_Scalar_Key = windowedScalarPoint(y_Inv, CT -> u, params);
		invKey = invertPoint(SK_Scalar_Key, params);

		plaintext = groupOp(CT -> v, invKey, params);
	}
	else
	{
		SK_Scalar_Key = windowedScalarPoint(sk, CT -> u, params);
		invKey = invertPoint(SK_Scalar_Key, params);

		invKeyY = windowedScalarPoint(y, invKey, params);

		plaintext = groupOp(CT -> v, invKeyY, params);

		clearECC_Point(invKeyY);
	}

	clearECC_Point(SK_Scalar_Key);
	clearECC_Point(invKey);

	return plaintext;
}


/*	----------------------------------------------------
	----    ----    ----TESTING LAND----    ----    ----
	----------------------------------------------------	*/





void testECC_Utils()
{
	mpz_t plaintext_x, plaintext_y, SK;
	struct eccPoint *plaintext, *plaintextDot, *PK, **preComputes;
	struct ecc_Ciphertext *ciphertext;
	struct eccParams *params;
	gmp_randstate_t *state = seedRandGen();
	int i;

	struct timespec timestamp_0, timestamp_1;
	clock_t c_0, c_1;


	mpz_init(SK);
	mpz_init_set_ui(plaintext_x, 19);
	mpz_init_set_ui(plaintext_y, 72);

	params = initBrainpool_256_Curve();
	plaintext = initAndSetECC_Point(plaintext_x, plaintext_y, 0);
	plaintextDot = initAndSetECC_Point(plaintext_x, plaintext_y, 0);

	mpz_urandomm(SK, *state, params -> n);



	timestamp_0 = timestamp();
	c_0 = clock();
	for(i = 0; i < 2000; i ++)
	{
		PK = windowedScalarPoint(SK, params -> g, params);
	}
	c_1 = clock();
	timestamp_1 = timestamp();
	printTiming(&timestamp_0, &timestamp_1, c_0, c_1, "Not fixed Test");

	printPoint(PK);

}







