struct eccPoint *initECC_Point()
{
	struct eccPoint *output = (struct eccPoint *) calloc(1, sizeof(struct eccPoint));

	mpz_init(output -> x);
	mpz_init(output -> y);

	output -> pointAtInf = 0;

	return output;
}


struct eccPoint *init_Identity_ECC_Point()
{
	struct eccPoint *output = (struct eccPoint *) calloc(1, sizeof(struct eccPoint));

	mpz_init(output -> x);
	mpz_init(output -> y);

	output -> pointAtInf = 1;

	return output;
}


struct eccPoint *initAndSetECC_Point(mpz_t x, mpz_t y, unsigned char identity)
{
	struct eccPoint *output = (struct eccPoint *) calloc(1, sizeof(struct eccPoint));

	mpz_init_set(output -> x, x);
	mpz_init_set(output -> y, y);

	output -> pointAtInf = identity;


	return output;
}


struct eccPoint *copyECC_Point(struct eccPoint *toCopy)
{
	struct eccPoint *output = (struct eccPoint *) calloc(1, sizeof(struct eccPoint));

	mpz_init_set(output -> x, toCopy -> x);
	mpz_init_set(output -> y, toCopy -> y);

	output -> pointAtInf = toCopy -> pointAtInf;

	return output;
}


void printPoint(struct eccPoint *P)
{
	if(0 == P -> pointAtInf)
	{
		gmp_printf("(%Zd, %Zd)\n", P -> x, P -> y);
	}
	else
	{
		printf("(@, @)\n");
	}
}


struct eccParams *initECC_Params()
{
	struct eccParams *output = (struct eccParams *) calloc(1, sizeof(struct eccParams));

	mpz_init(output -> p);
	mpz_init(output -> a);
	mpz_init(output -> b);
	output -> g = initECC_Point();
	mpz_init(output -> n);

	return output;
}


struct eccParams *initAndSetECC_Params(mpz_t p, mpz_t a, mpz_t b, struct eccPoint *g, mpz_t n)
{
	struct eccParams *output = (struct eccParams *) calloc(1, sizeof(struct eccParams));


	mpz_init_set(output -> p, p);
	mpz_init_set(output -> a, a);
	mpz_init_set(output -> b, b);
	mpz_init_set(output -> n, n);

	output -> g = g;


	return output;
}


struct ecc_Ciphertext *initECC_Ciphertext()
{
	struct ecc_Ciphertext *output = (struct ecc_Ciphertext *) calloc(1, sizeof(struct ecc_Ciphertext));

	output -> sessionKey = initECC_Point();

	mpz_init(output -> c1);
	mpz_init(output -> c2);


	return output;
}


struct eccPublicKey *initECC_PublicKey()
{
	struct eccPublicKey *output = (struct eccPublicKey *) calloc(1, sizeof(struct eccPublicKey));


	output -> basePK = initECC_Point();
	output -> PK_mul_SK = initECC_Point();


	return output;
}




