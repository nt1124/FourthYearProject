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


	mpz_init(SK);

	do
	{
		mpz_urandomm(SK, state, params -> p);
	} while( 0 != mpz_cmp_ui(SK, 0) );

	PK = scalarMulti(SK, params -> g, params);


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
		mpz_urandomm(k, state, params -> p);
	} while( 0 != mpz_cmp_ui(k, 0) );

	ciphertext -> sessionKey = scalarMulti(k, params -> g, params);
	tempPoint = scalarMulti(k, PK, params);
	ciphertext -> msgPart = groupOp(msg, tempPoint, params);


	free(tempPoint);

	return ciphertext;
}


struct eccPoint *ECC_Dec(mpz_t SK, struct eccPoint *plaintext, struct eccParams *params)
{
	// struct eccPoint *ciphertext;
	// mpz_t temp;


	// mpz_init(temp);
	// ciphertext = initECC_Point();

	// mpz_mul(temp, plaintext -> x, PK -> PK_mul_SK -> x);
	// mpz_mod(ciphertext -> x, temp, params -> p);
	// mpz_mul(temp, plaintext -> y, PK -> PK_mul_SK -> y);
	// mpz_mod(ciphertext -> y, temp, params -> p);


	// return ciphertext;
}





/*	----------------------------------------------------
	----    ----    ----++++8888++++----    ----    ----
	----------------------------------------------------	*/






void testECC_Utils()
{
	mpz_t p, a, b, n, g_x, g_y, plaintext_x, plaintext_y;
	mpz_t SK_a, SK_b, SK_j;
	struct eccPoint *g, *plaintext, *PK_a, *PK_b, *PK_j, *PK_for_use;
	struct eccParams *params;
	struct eccPublicKey *PK;
	struct eccPoint *ciphertext, *SK;

	mpz_t c_x, c_y, temp;

	mpz_init_set_ui(p, 263);
	mpz_init_set_ui(a, 1);
	mpz_init_set_ui(b, 1);
	mpz_init_set_ui(g_x, 184);
	mpz_init_set_ui(g_y, 220);
	mpz_init_set_ui(n, 64);
	mpz_init_set_ui(plaintext_x, 19);
	mpz_init_set_ui(plaintext_y, 72);

	mpz_init_set_ui(SK_a, 132);
	mpz_init_set_ui(SK_b, 155);
	mpz_init_set_ui(SK_j, 145);

	mpz_init(c_x);
	mpz_init(c_y);
	mpz_init(temp);


	g = initAndSetECC_Point(g_x, g_y, 0);
	plaintext = initAndSetECC_Point(plaintext_x, plaintext_y, 0);
	params = initAndSetECC_Params(p, a, b, g, n);
	/*
	PK = initECC_PublicKey();


	// Test the public key crap.
	PK_a = scalarMulti(SK_a, g, params);
	printPoint(PK_a);
	PK_b = scalarMulti(SK_b, g, params);
	printPoint(PK_b);
	PK_j = scalarMulti(SK_j, g, params);
	printPoint(PK_j);


	PK -> PK_FromOther = copyECC_Point(PK_b);
	PK -> PK_mul_SK = scalarMulti(SK_a, PK_b, params);

	SK = getDecryptionKey(SK, PK, params);

	ciphertext = ECC_Enc(PK, plaintext, params);

	printPoint(ciphertext);
	*/


}