/*
struct u_v_Pair *randomiseDDH(struct DDH_PK *pk, struct DDH_Group *group, gmp_randstate_t state)
{
	struct u_v_Pair *toReturn = (struct u_v_Pair*) calloc(1, sizeof(struct u_v_Pair));
	mpz_t s, t;
	mpz_t tempPowS, tempPowT;
	mpz_t temp;

	mpz_init(s);
	mpz_init(t);
	mpz_init(temp);
	mpz_init(tempPowS);
	mpz_init(tempPowT);
	mpz_init(toReturn -> u);
	mpz_init(toReturn -> v);

	mpz_urandomm(s, state, group -> p);
	mpz_urandomm(t, state, group -> p);

	mpz_powm(tempPowS, pk -> g, s, group -> p);
	mpz_powm(tempPowT, pk -> h, t, group -> p);
	mpz_mul(temp, tempPowS, tempPowT);
	mpz_mod(toReturn -> u, temp, group -> p);

	mpz_powm(tempPowS, pk -> g_x, s, group -> p);
	mpz_powm(tempPowT, pk -> h_x, t, group -> p);
	mpz_mul(temp, tempPowS, tempPowT);
	mpz_mod(toReturn -> v, temp, group -> p);

	return toReturn;
}
*/

struct eccPoint *generate_ECC_KeyPair(struct eccParams *params, mpz_t SK, gmp_randstate_t state)
{
	struct eccPoint *PK = initECC_Point();


	do
	{
		mpz_urandomm(SK, state, params -> n);
	} while( 0 == mpz_cmp_ui(SK, 0) );

	PK = doubleAndAdd_ScalarMul(SK, params -> g, params);


	return PK;
}


// http://crypto.stackexchange.com/questions/9987/elgamal-with-elliptic-curves
struct ecc_Ciphertext *ECC_Enc(struct eccPoint *PK, struct eccPoint *msg, struct eccParams *params, gmp_randstate_t state)
{
	struct ecc_Ciphertext *ciphertext = (struct ecc_Ciphertext *) calloc(1, sizeof(struct ecc_Ciphertext));
	struct eccPoint *tempPoint;
	mpz_t k;


	mpz_init(k);

	do
	{
		mpz_urandomm(k, state, params -> n);
	} while( 0 == mpz_cmp_ui(k, 0) );

	ciphertext -> sessionKey = doubleAndAdd_ScalarMul(k, params -> g, params);
	tempPoint = doubleAndAdd_ScalarMul(k, PK, params);
	ciphertext -> msgPart = groupOp(msg, tempPoint, params);


	mpz_clear(k);
	clearECC_Point(tempPoint);

	return ciphertext;
}


struct eccPoint *ECC_Dec(mpz_t SK, struct ecc_Ciphertext *ciphertext, struct eccParams *params)
{
	struct eccPoint *plaintext, *SK_Scalar_Key, *invKey;


	SK_Scalar_Key = doubleAndAdd_ScalarMul(SK, ciphertext -> sessionKey, params);

	invKey = invertPoint(SK_Scalar_Key, params);
	plaintext = groupOp(ciphertext -> msgPart, invKey, params);

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
	struct eccPoint *plaintext, *plaintextDot, *PK;
	struct ecc_Ciphertext *ciphertext;
	struct eccParams *params;
	gmp_randstate_t *state = seedRandGen();


	mpz_init(SK);
	mpz_init_set_ui(plaintext_x, 19);
	mpz_init_set_ui(plaintext_y, 72);

	plaintext = initAndSetECC_Point(plaintext_x, plaintext_y, 0);
	params = initBrainpool_256_Curve();


	PK = generate_ECC_KeyPair(params, SK, *state);
	ciphertext = ECC_Enc(PK, plaintext, params, *state);
	plaintextDot = ECC_Dec(SK, ciphertext, params);

	printPoint(plaintext);
	printPoint(plaintextDot);

}








