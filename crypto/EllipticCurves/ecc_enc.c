struct ecc_Ciphertext *ECC_Enc(struct eccPublicKey *PK, struct eccPoint *plaintext, struct eccParams *params)
{
	struct eccCiphertext *ciphertext;
	mpz_t temp;


	mpz_init(temp);
	ciphertext = initECC_Point();

	mpz_mul(temp, plaintext -> x, PK -> PK_mul_SK -> x);
	mpz_mod(ciphertext -> x, temp, params -> p);
	mpz_mul(temp, plaintext -> y, PK -> PK_mul_SK -> y);
	mpz_mod(ciphertext -> y, temp, params -> p);


	return ciphertext;
}


struct eccPoint *ECC_Dec(mpz_t SK, struct eccPoint *plaintext, struct eccParams *params)
{
	struct eccPoint *ciphertext;
	mpz_t temp;


	mpz_init(temp);
	ciphertext = initECC_Point();

	mpz_mul(temp, plaintext -> x, PK -> PK_mul_SK -> x);
	mpz_mod(ciphertext -> x, temp, params -> p);
	mpz_mul(temp, plaintext -> y, PK -> PK_mul_SK -> y);
	mpz_mod(ciphertext -> y, temp, params -> p);


	return ciphertext;
}





/*	----------------------------------------------------
	----    ----    ----++++8888++++----    ----    ----
	----------------------------------------------------	*/






int testECC_Utils()
{
	mpz_t p, a, b, n, g_x, g_y, plaintext_x, plaintext_y;
	mpz_t SK_a, SK_b, SK_j;
	struct eccPoint *g, *plaintext, *ciphertext;
	struct eccPoint *PK_a, *PK_b, *PK_j, *PK_for_use;
	struct eccParams *params;

	mpz_t c_x, c_y, temp;

	mpz_init_set_ui(p, 263);
	mpz_init_set_ui(a, 1);
	mpz_init_set_ui(b, 1);
	mpz_init_set_ui(g_x, 219);
	mpz_init_set_ui(g_y, 118);
	mpz_init_set_ui(n, 64);
	mpz_init_set_ui(plaintext_x, 19);
	mpz_init_set_ui(plaintext_y, 72);

	mpz_init_set_ui(SK_a, 103);
	mpz_init_set_ui(SK_b, 205);
	mpz_init_set_ui(SK_j, 209);

	mpz_init(c_x);
	mpz_init(c_y);
	mpz_init(temp);


	g = initAndSetECC_Point(g_x, g_y, 0);
	plaintext = initAndSetECC_Point(plaintext_x, plaintext_y, 0);
	params = initAndSetECC_Params(p, a, b, g, n);


	// Test the public key crap.
	PK_a = scalarMulti(SK_a, g, params);
	printPoint(PK_a);
	PK_b = scalarMulti(SK_b, g, params);
	printPoint(PK_b);
	PK_j = scalarMulti(SK_j, g, params);
	printPoint(PK_j);

	ciphertext = initECC_Point();
	PK_for_use = scalarMulti(SK_a, PK_b, params);
	mpz_mul(temp, plaintext -> x, PK_for_use -> x);
	mpz_mod(ciphertext -> x, temp, params -> p);
	mpz_mul(temp, plaintext -> y, PK_for_use -> y);
	mpz_mod(ciphertext -> y, temp, params -> p);

	printPoint(ciphertext);

	return 0;
}