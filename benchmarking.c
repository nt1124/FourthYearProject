void benchmarkCommunications()
{
	randctx *isaacCTX;
	unsigned char *commBuffer;
	int commBufferLen;

	isaacCTX = (randctx*) calloc(1, sizeof(randctx));
	getIsaacContext(isaacCTX);


}



void benchmarkECC()
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
	preComputes = preComputePoints(params -> g, 512, params);
	for(i = 0; i < 2000; i ++)
	{
		PK = windowedScalarFixedPoint(SK, params -> g, preComputes, 9, params);
	}
	c_1 = clock();
	timestamp_1 = timestamp();
	printTiming(&timestamp_0, &timestamp_1, c_0, c_1, "Fixed Test 10");
	printPoint(PK);


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

