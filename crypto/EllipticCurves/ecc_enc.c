
struct u_v_Pair_ECC *randomiseDDH_ECC(struct ECC_PK *pk, struct eccParams *params, gmp_randstate_t state)
{
	struct u_v_Pair_ECC *toReturn = (struct u_v_Pair_ECC*) calloc(1, sizeof(struct u_v_Pair_ECC));
	struct eccPoint *tempScalarS, *tempScalarT;
	mpz_t s, t;
	mpz_t tempPowS, tempPowT;
	mpz_t temp;


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


// http://crypto.stackexchange.com/questions/9987/elgamal-with-elliptic-curves
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

/*
struct eccPoint *ECC_Dec_Alt(mpz_t sk, mpz_t y, struct eccParams *params, struct u_v_Pair_ECC *C, unsigned char sigmaBit)
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
		mpz_invert(y_Inv, y, group -> q);

		mpz_mul(finalFactor, y_Inv, sk);
		// mpz_mul(y_Inv, finalFactor, group -> q);

		// mpz_powm(c1_sk, C -> u, finalFactor, group -> p);
		// mpz_invert(c1_sk_inv, c1_sk, group -> p);

	}
	else
	{
		SK_Scalar_Key = windowedScalarPoint(SK, ciphertext -> u, params);
		invKey = invertPoint(SK_Scalar_Key, params);

		invKeyY = windowedScalarPoint(y, invKey, params);

		plaintext = groupOp(ciphertext -> v, invKeyY, params);
	}

	clearECC_Point(SK_Scalar_Key);
	clearECC_Point(invKey);
	clearECC_Point(invKeyY);

	return plaintext;
}
*/

/*	----------------------------------------------------
	----    ----    ----TESTING LAND----    ----    ----
	----------------------------------------------------	*/





void testECC_Utils()
{
	mpz_t plaintext_x, plaintext_y, SK;
	struct eccPoint *plaintext, *plaintextDot, *PK;
	struct ecc_Ciphertext *ciphertext;
	struct eccParams *params;
	gmp_randstate_t *state = seedRandGen();

	mpz_init(SK);
	mpz_init_set_ui(plaintext_x, 19);
	mpz_init_set_ui(plaintext_y, 72);

	plaintext = initAndSetECC_Point(plaintext_x, plaintext_y, 0);
	params = initBrainpool_256_Curve();

	mpz_urandomm(plaintext_y, *state, params -> n);
	plaintextDot = windowedScalarPoint(plaintext_y, plaintext, params);

	PK = groupOp(plaintext, plaintextDot, params);
	groupOp_PlusEqual(plaintext, plaintextDot, params);

	printPoint(PK);
	printPoint(plaintext);

	/*

	PK = generate_ECC_KeyPair(params, SK, *state);
	ciphertext = ECC_Enc(PK, plaintext, params, *state);
	plaintextDot = ECC_Dec(SK, ciphertext, params);

	printPoint(plaintext);
	printPoint(plaintextDot);

	*/

}







