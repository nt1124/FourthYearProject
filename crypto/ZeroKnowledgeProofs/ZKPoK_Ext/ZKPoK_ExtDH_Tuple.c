 struct twoDH_Tuples *initTwoDH_Tuples(struct eccPoint *g_0, struct eccPoint *g_1,
									struct eccPoint *h_0, struct eccPoint *h_1,
									struct eccPoint *u, struct eccPoint *v)
{
	struct twoDH_Tuples *toReturn = (struct twoDH_Tuples*) calloc(1, sizeof(struct twoDH_Tuples));
	
	toReturn -> g_0_List = (struct eccPoint **) calloc(2, sizeof(struct eccPoint*));
	toReturn -> g_1_List = (struct eccPoint **) calloc(2, sizeof(struct eccPoint*));
	toReturn -> h_0_List = (struct eccPoint **) calloc(2, sizeof(struct eccPoint*));
	toReturn -> h_1_List = (struct eccPoint **) calloc(2, sizeof(struct eccPoint*));

	toReturn -> g_0_List[0] = g_0;
	toReturn -> g_0_List[1] = g_1;

	toReturn -> g_1_List[0] = u;
	toReturn -> g_1_List[1] = u;

	toReturn -> h_0_List[0] = h_0;
	toReturn -> h_0_List[1] = h_1;

	toReturn -> h_1_List[0] = v;
	toReturn -> h_1_List[1] = v;


	return toReturn;
}


// witness is basically r_j.
// g_0 is g from params, g_1 is g_1.
// h_0, h_1 are the entry from the CRSs h_0_List and h_1_List
// All u_i are equal and are g_tilde
// v_i is the i^th entry in the tildeList
void ZKPoK_Ext_DH_TupleProver(int writeSocket, int readSocket, int stat_SecParam, mpz_t *witness, unsigned char *J_set,
							struct eccPoint *g_0, struct eccPoint *g_1,
							struct eccPoint *h_0, struct eccPoint *h_1,
							struct eccPoint **u_array, struct eccPoint **v_array,
							struct eccParams *params, gmp_randstate_t *state)
{
	struct eccPoint *uProduct, *vProduct, *u_temp, *v_temp;
	struct twoDH_Tuples *tuples;

	mpz_t *lambda;
	unsigned char *commBuffer;
	int i, verified = 0, commBufferLen = 0, bufferOffset = 0;


	uProduct = init_Identity_ECC_Point();
	vProduct = init_Identity_ECC_Point();


	commBuffer = receiveBoth(readSocket, commBufferLen);
	lambda = deserialiseMPZ_Array(commBuffer, &bufferOffset);


	for(i = 0; i < stat_SecParam; i ++)
	{
		u_temp = windowedScalarPoint(lambda[i], u_array[i], params);
		v_temp = windowedScalarPoint(lambda[i], v_array[i], params);

		groupOp_PlusEqual(uProduct, u_temp, params);
		groupOp_PlusEqual(vProduct, v_temp, params);

		clearECC_Point(u_temp);
		clearECC_Point(v_temp);
	}


	tuples = initTwoDH_Tuples(g_0, g_1, h_0, h_1, uProduct, vProduct);
	ZKPoK_Prover_ECC_1Of2(writeSocket, readSocket, params,
					tuples -> g_0_List, tuples -> g_1_List,
					tuples -> h_0_List, tuples -> h_1_List,
					witness, J_set, state);

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