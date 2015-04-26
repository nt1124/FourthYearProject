struct eccPoint *setup_OT_NP_Sender(struct eccParams *params, gmp_randstate_t state)
{
	struct eccPoint *C;
	mpz_t y;


	mpz_init(y);
	do
	{
		mpz_urandomm(y, state, params -> n);
	} while(0 == mpz_cmp_ui(y, 0));

	C = fixedPointMultiplication(gPreComputes, y, params);

	mpz_clear(y);

	return C;
}


struct OT_NP_Receiver_Query *OT_NP_query(struct eccPoint *C, unsigned char inputBit, struct eccParams *params, gmp_randstate_t state)
{
	struct OT_NP_Receiver_Query *query = init_OT_NP_Receiver_Query();
	struct eccPoint *g_k, *inv_g_k;


	do
	{
		mpz_urandomm(query -> k, state, params -> n);
	} while(0 == mpz_cmp_ui(query -> k, 0));


	g_k = fixedPointMultiplication(gPreComputes, query -> k, params);

	if(0x00 == inputBit)
	{
		query -> h = copyECC_Point(g_k);
	}
	else
	{
		inv_g_k = invertPoint(g_k, params);
		query -> h = groupOp(C, inv_g_k, params);

		clearECC_Point(inv_g_k);
	}

	clearECC_Point(g_k);


	return query;
}



unsigned char *OT_NP_enc_0(struct eccPoint *h_r, unsigned char *x_0, int msgLength, struct eccParams *params)
{
	unsigned char *c_0, *hashed_h_r, *h_r_xBytes;
	int tempLength = 0;


	h_r_xBytes = convertMPZToBytes(h_r -> x, &tempLength);
	hashed_h_r = sha256_full(h_r_xBytes, tempLength);
	c_0 = XOR_TwoStrings(hashed_h_r, x_0, msgLength);


	return c_0;
}


unsigned char *OT_NP_enc_1(struct eccPoint *C, struct eccPoint *h, mpz_t r, unsigned char *x_1, int msgLength, struct eccParams *params)
{
	unsigned char *c_1, *hashed_C_h_r, *C_h_r_xBytes;
	struct eccPoint *C_h, *C_h_r, *inv_h;
	int tempLength = 0;


	inv_h = invertPoint(h, params);
	C_h = groupOp(C, inv_h, params);
	C_h_r = windowedScalarPoint(r, C_h, params);

	C_h_r_xBytes = convertMPZToBytes(C_h_r -> x, &tempLength);
	hashed_C_h_r = sha256_full(C_h_r_xBytes, tempLength);
	c_1 = XOR_TwoStrings(hashed_C_h_r, x_1, msgLength);


	return c_1;
}



struct OT_NP_Sender_Transfer *OT_NP_Transfer(struct eccPoint *C, struct eccPoint *h, unsigned char *x_0, unsigned char *x_1,
											int msgLength, gmp_randstate_t state, struct eccParams *params)
{
	struct OT_NP_Sender_Transfer *transferOutput = init_OT_NP_Sender_Transfer();
	struct eccPoint *g_r, *h_r;
	mpz_t r;
	int tempLength = 0, tempOffset = 0;


	mpz_init(r);

	do
	{
		mpz_urandomm(r, state, params -> n);
	} while(0 == mpz_cmp_ui(r, 0));

	// transferOutput -> a = windowedScalarPoint(r, params -> g, params);
	transferOutput -> a = fixedPointMultiplication(gPreComputes, r, params);
	h_r = windowedScalarPoint(r, h, params);
	
	transferOutput -> c_0 = OT_NP_enc_0(h_r, x_0, 16, params);
	transferOutput -> c_1 = OT_NP_enc_1(C, h, r, x_1, 16, params);


	return transferOutput;
}



unsigned char *OT_NP_Output_Xb(struct eccPoint *C, struct eccPoint *a, mpz_t k,
							unsigned char *c_0, unsigned char *c_1, unsigned char inputBit,
							int msgLength, struct eccParams *params)
{
	unsigned char *x_b, *a_k_xBytes, *hashedBytes;
	struct eccPoint *a_k;
	int tempLength = 0;


	a_k = windowedScalarPoint(k, a, params);
	a_k_xBytes = convertMPZToBytes(a_k -> x, &tempLength);
	hashedBytes = sha256_full(a_k_xBytes, tempLength);


	if(0x00 == inputBit)
	{
		x_b = XOR_TwoStrings(hashedBytes, c_0, msgLength);
	}
	else
	{
		x_b = XOR_TwoStrings(hashedBytes, c_1, msgLength);
	}
 

	return x_b;
}



void test_local_OT_NP(int sigmaBitInt)
{
	struct eccParams *params;
	struct eccPoint *C, *a;
	struct OT_NP_Receiver_Query **queries_R;
	struct OT_NP_Sender_Transfer **transfers_R;
	struct eccPoint **queries_S;
	struct OT_NP_Sender_Transfer **transfers_S;


	int i, j, k, numInputs = 8, numTests = 4, comp_SecParam = 1024;
	int totalOTs = numInputs * numTests;


	unsigned char *inputBytes[numTests][2];
	unsigned char *outputBytes[numTests];
	unsigned char *commBuffer;
	unsigned char sigmaBit = (unsigned char) sigmaBitInt;

	int bufferOffset = 0, u_v_index = 0, tempInt = 0;

	gmp_randstate_t *state = seedRandGen();


	params = initBrainpool_256_Curve();


	for(i = 0; i < numTests; i ++)
	{
		inputBytes[i][0] = generateRandBytes(16, 16);
		inputBytes[i][1] = generateRandBytes(16, 16);
	}


	C = setup_OT_NP_Sender(params, *state);


	queries_R = (struct OT_NP_Receiver_Query **) calloc(numTests, sizeof(struct OT_NP_Receiver_Query *));
	for(i = 0; i < numTests; i ++)
	{
		queries_R[i] = OT_NP_query(C, sigmaBit, params, *state);
	}

	commBuffer = serialiseQueries(queries_R, numTests, &bufferOffset);
	queries_S = deserialiseQueries(commBuffer, numTests);
	free(commBuffer);

	transfers_S = (struct OT_NP_Sender_Transfer **) calloc(numTests, sizeof(struct OT_NP_Sender_Transfer *));
	for(i = 0; i < numTests; i ++)
	{
		transfers_S[i] = OT_NP_Transfer(C, queries_S[i], inputBytes[i][0], inputBytes[i][1], 16, *state, params);
	}

	bufferOffset = 0;
	commBuffer = serialiseTransferStructs(transfers_S, numTests, 16, &bufferOffset);
	transfers_R = deserialiseTransferStructs(commBuffer, numTests, 16);

	for(i = 0; i < numTests; i ++)
	{
		bufferOffset = 0;
		outputBytes[i] = OT_NP_Output_Xb(C, transfers_R[i] -> a, queries_R[i] -> k, transfers_R[i] -> c_0, transfers_R[i] -> c_1, sigmaBit, 16, params);
	}


	for(i = 0; i < numTests; i ++)
	{
		for(j = 0; j < 16; j ++)
		{
			printf("%02X", inputBytes[i][0][j]);
		}
		printf("\n");
	}

	printf("\n");

	for(i = 0; i < numTests; i ++)
	{
		for(j = 0; j < 16; j ++)
		{
			printf("%02X", inputBytes[i][1][j]);
		}
		printf("\n");
	}

	printf("\n");

	for(i = 0; i < numTests; i ++)
	{
		for(j = 0; j < 16; j ++)
		{
			printf("%02X", outputBytes[i][j]);
		}
		printf("\n");
	}

}

