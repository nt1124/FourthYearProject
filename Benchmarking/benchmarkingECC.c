void benchmarkECC_Exponentiation()
{
	mpz_t *expList, SK;
	struct eccPoint *plaintext, *plaintextDot, *PK, **preComputes;
	struct ecc_Ciphertext *ciphertext;
	struct eccParams *params;
	gmp_randstate_t *state = seedRandGen();
	int i;
	const int numExps = 2000;


	struct timespec timestamp_0, timestamp_1;
	clock_t c_0, c_1;


	mpz_init(SK);

	params = initBrainpool_256_Curve();

	mpz_urandomm(SK, *state, params -> n);

	expList = (mpz_t *) calloc(numExps, sizeof(mpz_t));
	for(i = 0; i < numExps; i ++)
	{
		mpz_init(expList[i]);
		mpz_urandomm(expList[i], *state, params -> n);
	}


	printf("Number of Exponentiations = %d\n", numExps);


	timestamp_0 = timestamp();
	c_0 = clock();

	preComputes = preComputePoints(params -> g, 512, params);

	for(i = 0; i < numExps; i ++)
	{
		PK = windowedScalarFixedPoint(SK, params -> g, preComputes, 9, params);
	}

	c_1 = clock();
	timestamp_1 = timestamp();
	printTiming(&timestamp_0, &timestamp_1, c_0, c_1, "Fixed Point Exponentiation Test");

	timestamp_0 = timestamp();
	c_0 = clock();

	for(i = 0; i < numExps; i ++)
	{
		PK = windowedScalarPoint(SK, params -> g, params);
	}

	c_1 = clock();
	timestamp_1 = timestamp();
	printTiming(&timestamp_0, &timestamp_1, c_0, c_1, "Standard Exponentiation Test");

}

