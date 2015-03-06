// witness is basically r_j.
// g_0 is g from params, g_1 is g_1.
// h_0, h_1 are the entry from the CRSs h_0_List and h_1_List
// All u_i are equal and are g_tilde
// v_i is the i^th entry in the tildeList
void ZKPoK_Ext_DH_TupleProver(int writeSocket, int readSocket, int stat_SecParam,
							mpz_t *witness, struct eccPoint *g_1, struct eccPoint *h_0, struct eccPoint *h_1,
							struct tildeList *tildeList, struct eccParams *params,
							gmp_randstate_t *state)
{
	struct eccPoint *uProduct, *vProduct;
	struct eccPoint **v_array = (struct eccPoint **) calloc(stat_SecParam, sizeof(struct eccPoint*));

	mpz_t *lambda, lambdaSum;
	unsigned char *commBuffer;
	int i, verified = 0, commBufferLen = 0, bufferOffset = 0;


	uProduct = init_Identity_ECC_Point();
	vProduct = init_Identity_ECC_Point();
	mpz_init_set_ui(lambdaSum, 0);


	commBuffer = receiveBoth(readSocket, commBufferLen);
	lambda = deserialiseMPZ_Array(commBuffer, &bufferOffset);


	for(i = 0; i < stat_SecParam; i ++)
	{
		mpz_add(lambdaSum, lambdaSum, lambda[i]);
		v_array[i] = windowedScalarPoint(lambda[i], tildeList -> h_tildeList[i], params);
		groupOp_PlusEqual(vProduct, v_array[i], params);
	}

	uProduct = windowedScalarPoint(lambdaSum, tildeList -> g_tilde, params);

	/*
	ZKPoK_Prover_ECC(writeSocket, readSocket, params, stat_SecParam, g_0, g_1, **h_0_List, **h_1_List,
					witness, {0, 1}, state)
	*/
}


int ZKPoK_Ext_DH_TupleVerifier(int writeSocket, int readSocket, int stat_SecParam,
							struct eccPoint *g_1, struct eccPoint *h_0, struct eccPoint *h_1,
							struct tildeList *tildeList, struct eccParams *params,
							gmp_randstate_t *state)
{
	struct eccPoint *uProduct, *vProduct;
	struct eccPoint **v_array = (struct eccPoint **) calloc(stat_SecParam, sizeof(struct eccPoint*));
	mpz_t *lambda = (mpz_t*) calloc(stat_SecParam, sizeof(mpz_t)), lambdaSum;
	unsigned char *commBuffer;
	int i, verified = 0, commBufferLen = 0, bufferOffset = 0;


	for(i = 0; i < stat_SecParam; i ++)
	{
		mpz_init(lambda[i]);
		mpz_urandomm(lambda[i], *state, params -> n);
	}


	commBuffer = serialiseMPZ_Array(lambda, stat_SecParam, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);


	for(i = 0; i < stat_SecParam; i ++)
	{
		mpz_add(lambdaSum, lambdaSum, lambda[i]);
		v_array[i] = windowedScalarPoint(lambda[i], tildeList -> h_tildeList[i], params);
		groupOp_PlusEqual(vProduct, v_array[i], params);
	}

	uProduct = windowedScalarPoint(lambdaSum, tildeList -> g_tilde, params);


}