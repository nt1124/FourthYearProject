struct u_v_Pair_ECC_Hom *randomiseDDH_ECC_Hom(struct ECC_PK_Hom *pk, struct eccParams_Hom *params, gmp_randstate_t state)
{
	struct u_v_Pair_ECC_Hom *toReturn = (struct u_v_Pair_ECC_Hom*) calloc(1, sizeof(struct u_v_Pair_ECC_Hom));
	struct eccPoint_Hom *tempScalarS, *tempScalarT;
	mpz_t s, t;
	mpz_t tempPowS, tempPowT;
	mpz_t temp;


	mpz_init(s);
	mpz_init(t);

	mpz_urandomm(s, state, params -> n);
	mpz_urandomm(t, state, params -> n);

	tempScalarS = windowedScalarPoint_Hom(s, pk -> g, params);
	tempScalarT = windowedScalarPoint_Hom(t, pk -> h, params);
	toReturn -> u = groupOp_Hom(tempScalarS, tempScalarT, params);
	clearECC_Point_Hom(tempScalarS);
	clearECC_Point_Hom(tempScalarT);


	tempScalarS = windowedScalarPoint_Hom(s, pk -> g_x, params);
	tempScalarT = windowedScalarPoint_Hom(t, pk -> h_x, params);
	toReturn -> v = groupOp_Hom(tempScalarS, tempScalarT, params);
	clearECC_Point_Hom(tempScalarS);
	clearECC_Point_Hom(tempScalarT);


	return toReturn;
}

struct eccPoint_Hom *generate_ECC_KeyPair_Hom(struct eccParams_Hom *params, mpz_t SK, gmp_randstate_t state)
{
	struct eccPoint_Hom *PK = initECC_Point_Hom();


	do
	{
		mpz_urandomm(SK, state, params -> n);
	} while( 0 == mpz_cmp_ui(SK, 0) );

	PK = windowedScalarPoint_Hom(SK, params -> g, params);


	return PK;
}


// http://crypto.stackexchange.com/questions/9987/elgamal-with-elliptic-curves
struct u_v_Pair_ECC_Hom *ECC_Enc_Hom(struct ECC_PK_Hom *pk, struct eccPoint_Hom *msg, struct eccParams_Hom *params, gmp_randstate_t state)
{
	struct u_v_Pair_ECC_Hom *ciphertext = (struct u_v_Pair_ECC_Hom *) calloc(1, sizeof(struct u_v_Pair_ECC_Hom));
	struct eccPoint_Hom *tempPoint;


	ciphertext = randomiseDDH_ECC_Hom(pk, params, state);
	tempPoint = groupOp_Hom(ciphertext -> v, msg, params);

	clearECC_Point_Hom(ciphertext -> v);
	ciphertext -> v = tempPoint;


	return ciphertext;
}


struct eccPoint_Hom *ECC_Dec_Hom(mpz_t SK, struct u_v_Pair_ECC_Hom *ciphertext, struct eccParams_Hom *params)
{
	struct eccPoint_Hom *plaintext, *SK_Scalar_Key, *invKey;


	SK_Scalar_Key = windowedScalarPoint_Hom(SK, ciphertext -> u, params);

	invKey = invertPoint_Hom(SK_Scalar_Key, params);
	plaintext = groupOp_Hom(ciphertext -> v, invKey, params);

	clearECC_Point_Hom(SK_Scalar_Key);
	clearECC_Point_Hom(invKey);


	return plaintext;
}





/*	----------------------------------------------------
	----    ----    ----TESTING LAND----    ----    ----
	----------------------------------------------------	*/





void testECC_Utils_Hom()
{
	mpz_t plaintext_x, plaintext_y, plaintext_z, SK, *tempMPZ;
	struct eccPoint_Hom *plaintext, *plaintextDot, *PK;
	struct u_v_Pair_ECC_Hom *ciphertext;
	struct eccParams_Hom *params;
	gmp_randstate_t *state = seedRandGen();
	int i;

	mpz_init_set_ui(SK, 14);
	mpz_init_set_ui(plaintext_x, 19);
	mpz_init_set_ui(plaintext_y, 72);
	mpz_init_set_ui(plaintext_z, 1);

	plaintext = initAndSetECC_Point_Hom(plaintext_x, plaintext_y, plaintext_z, 0);
	params = initBrainpool_256_Curve_Hom();
	struct eccParams *other_params = initBrainpool_256_Curve();

	struct eccPoint *other_plaintext = initAndSetECC_Point(plaintext_x, plaintext_y, 0);
	struct eccPoint *other_plaintext_dot;

	mpz_set_ui(plaintext_z, 2);


	struct timespec timestamp_0 = timestamp(), timestamp_1;
	clock_t c_0, c_1;
	c_0 = clock();

	mpz_urandomm(plaintext_x, *state, params -> p);

	for(i = 0; i < 4000; i ++)
	{
		plaintextDot = windowedScalarPoint_Hom(plaintext_x, plaintext, params);
		// plaintextDot = doubleAndAdd_ScalarMul_Hom(plaintext_x, plaintext, params);
		clearECC_Point_Hom(plaintext);
		plaintext = plaintextDot;
	}

	c_1 = clock();
	timestamp_1 = timestamp();

	printTiming(&timestamp_0, &timestamp_1, c_0, c_1, "Hom test");

	printPoint_Hom(plaintext);

	timestamp_0 = timestamp();
	c_0 = clock();


	for(i = 0; i < 4000; i ++)
	{
		other_plaintext_dot = windowedScalarPoint(plaintext_x, other_plaintext, other_params);
		// other_plaintext_dot = doubleAndAdd_ScalarMul(plaintext_x, other_plaintext, other_params);
		clearECC_Point(other_plaintext);
		other_plaintext = other_plaintext_dot;
	}

	c_1 = clock();
	timestamp_1 = timestamp();

	printTiming(&timestamp_0, &timestamp_1, c_0, c_1, "Old Test");

	printPoint(other_plaintext);
	struct eccPoint *converted = convertToAffineRep(plaintext, params);

	printPoint(converted);
	/*
	struct eccPoint_Hom *pkPoint = generate_ECC_KeyPair_Hom(params, SK, *state);

	ciphertext = ECC_Enc_Hom(pkPoint, plaintext, params, *state);

	PK = ECC_Dec_Hom(SK, ciphertext, params);
	*/
}







