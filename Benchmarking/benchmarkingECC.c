void benchmarkECC_Exponentiation()
{
	mpz_t *expList;
	struct eccPoint *plaintext, *plaintextDot, *PK, **preComputes;
	struct ecc_Ciphertext *ciphertext;
	struct eccParams *params;
	gmp_randstate_t *state = seedRandGen();
	int i;
	const int numExps = 5000;


	struct timespec timestamp_0, timestamp_1;
	clock_t c_0, c_1;


	params = initBrainpool_256_Curve();

	expList = (mpz_t *) calloc(numExps, sizeof(mpz_t));
	for(i = 0; i < numExps; i ++)
	{
		mpz_init(expList[i]);
		mpz_urandomm(expList[i], *state, params -> n);
	}


	printf("Number of Exponentiations = %d\n", numExps);


	timestamp_0 = timestamp();
	c_0 = clock();

	for(i = 0; i < numExps; i ++)
	{
		PK = windowedScalarPoint(expList[i], params -> g, params);
	}

	c_1 = clock();
	timestamp_1 = timestamp();
	printTiming(&timestamp_0, &timestamp_1, c_0, c_1, "Standard Exponentiation Test");




	timestamp_0 = timestamp();
	c_0 = clock();

	struct eccPoint **localPreComputes = fixedBasePreComputes(params -> g, params);

	for(i = 0; i < numExps; i ++)
	{
		PK = fixedPointMultiplication(localPreComputes, expList[i], params);
	}


	c_1 = clock();
	timestamp_1 = timestamp();
	printTiming(&timestamp_0, &timestamp_1, c_0, c_1, "New Fixed Point Exponentiation Test");
	printf("Note: Fixed point exp timings includes time to perform precomputes\n");
}




void benchmarkECC_Doubling()
{
	mpz_t tempExp;
	struct eccPoint *tempPoint, **outputArrays;
	struct eccParams *params;
	gmp_randstate_t *state;

	const int numDoubles = 50000;
	int i;


	struct timespec timestamp_0, timestamp_1;
	clock_t c_0, c_1;


	mpz_init(tempExp);

	state = seedRandGen();
	params = initBrainpool_256_Curve();


	outputArrays = (struct eccPoint **) calloc(numDoubles, sizeof(struct eccPoint *));


	printf("Number of Doublings In Place = %d\n", numDoubles);
	tempPoint = copyECC_Point(params -> g);


	timestamp_0 = timestamp();
	c_0 = clock();

	for(i = 0; i < numDoubles; i ++)
	{
		doublePointInPlace(tempPoint, params);
	}

	c_1 = clock();
	timestamp_1 = timestamp();
	printTiming(&timestamp_0, &timestamp_1, c_0, c_1, "Doubling Points In Place Test");


	printf("Number of Doublings Out Of Place = %d\n", numDoubles);
	outputArrays[0] = copyECC_Point(params -> g);

	timestamp_0 = timestamp();
	c_0 = clock();

	for(i = 1; i < numDoubles; i ++)
	{
		outputArrays[i] = doublePoint(outputArrays[i - 1], params);
	}
	doublePoint(outputArrays[numDoubles - 1], params);

	c_1 = clock();
	timestamp_1 = timestamp();
	printTiming(&timestamp_0, &timestamp_1, c_0, c_1, "Doubling Points In Place Test");

}




void benchmarkECC_GroupOp()
{
	struct eccPoint *tempPoint, **outputArrays;
	struct eccParams *params;
	gmp_randstate_t *state;

	const int numDoubles = 50000;
	int i;


	struct timespec timestamp_0, timestamp_1;
	clock_t c_0, c_1;



	state = seedRandGen();
	params = initBrainpool_256_Curve();


	outputArrays = (struct eccPoint **) calloc(numDoubles, sizeof(struct eccPoint *));


	printf("Number of Group Ops In Place = %d\n", numDoubles);
	tempPoint = copyECC_Point(params -> g);


	timestamp_0 = timestamp();
	c_0 = clock();

	for(i = 0; i < numDoubles; i ++)
	{
		groupOp_PlusEqual(tempPoint, params -> g, params);
	}

	c_1 = clock();
	timestamp_1 = timestamp();
	printTiming(&timestamp_0, &timestamp_1, c_0, c_1, "Additions In Place Test");


	printf("Number of Group Ops Out Of Place = %d\n", numDoubles);
	outputArrays[0] = copyECC_Point(params -> g);

	timestamp_0 = timestamp();
	c_0 = clock();

	for(i = 1; i < numDoubles; i ++)
	{
		outputArrays[i] = groupOp(outputArrays[i - 1], params -> g, params);
	}
	outputArrays[numDoubles - 1] = groupOp(outputArrays[numDoubles - 2], params -> g, params);

	c_1 = clock();
	timestamp_1 = timestamp();
	printTiming(&timestamp_0, &timestamp_1, c_0, c_1, "Adding Points In Place Test");

}

