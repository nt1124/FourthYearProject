struct twoDH_Tuples *initTwoDH_Tuples(struct eccPoint *g_0, struct eccPoint *g_1,
									struct eccPoint *h_0, struct eccPoint *h_1,
									struct eccPoint *u, struct eccPoint *v)
{
	struct twoDH_Tuples *toReturn = (struct twoDH_Tuples*) calloc(1, sizeof(struct twoDH_Tuples));
	
	toReturn -> g_0_List = (struct eccPoint **) calloc(2, sizeof(struct eccPoint*));
	toReturn -> g_1_List = (struct eccPoint **) calloc(2, sizeof(struct eccPoint*));
	toReturn -> h_0_List = (struct eccPoint **) calloc(2, sizeof(struct eccPoint*));
	toReturn -> h_1_List = (struct eccPoint **) calloc(2, sizeof(struct eccPoint*));

	toReturn -> g_0_List[0] = copyECC_Point(g_0);
	toReturn -> g_0_List[1] = copyECC_Point(g_1);

	toReturn -> g_1_List[0] = copyECC_Point(u);
	toReturn -> g_1_List[1] = copyECC_Point(u);

	toReturn -> h_0_List[0] = copyECC_Point(h_0);
	toReturn -> h_0_List[1] = copyECC_Point(h_1);

	toReturn -> h_1_List[0] = copyECC_Point(v);
	toReturn -> h_1_List[1] = copyECC_Point(v);


	return toReturn;
}

 struct twoDH_Tuples *initTwoDH_Tuples_2U(struct eccPoint *g_0, struct eccPoint *g_1,
										struct eccPoint *h_0, struct eccPoint *h_1,
										struct eccPoint *u_0, struct eccPoint *u_1,
										struct eccPoint *v)
{
	struct twoDH_Tuples *toReturn = (struct twoDH_Tuples*) calloc(1, sizeof(struct twoDH_Tuples));
	
	toReturn -> g_0_List = (struct eccPoint **) calloc(2, sizeof(struct eccPoint*));
	toReturn -> g_1_List = (struct eccPoint **) calloc(2, sizeof(struct eccPoint*));
	toReturn -> h_0_List = (struct eccPoint **) calloc(2, sizeof(struct eccPoint*));
	toReturn -> h_1_List = (struct eccPoint **) calloc(2, sizeof(struct eccPoint*));

	toReturn -> g_0_List[0] = copyECC_Point(g_0);
	toReturn -> g_0_List[1] = copyECC_Point(g_1);

	toReturn -> g_1_List[0] = copyECC_Point(u_0);
	toReturn -> g_1_List[1] = copyECC_Point(u_1);

	toReturn -> h_0_List[0] = copyECC_Point(h_0);
	toReturn -> h_0_List[1] = copyECC_Point(h_1);

	toReturn -> h_1_List[0] = copyECC_Point(v);
	toReturn -> h_1_List[1] = copyECC_Point(v);


	return toReturn;
}

 struct twoDH_Tuples *initTwoDH_Tuples_2U_2V(struct eccPoint *g_0, struct eccPoint *g_1,
										struct eccPoint *h_0, struct eccPoint *h_1,
										struct eccPoint *u_0, struct eccPoint *u_1,
										struct eccPoint *v_0, struct eccPoint *v_1)
{
	struct twoDH_Tuples *toReturn = (struct twoDH_Tuples*) calloc(1, sizeof(struct twoDH_Tuples));
	
	toReturn -> g_0_List = (struct eccPoint **) calloc(2, sizeof(struct eccPoint*));
	toReturn -> g_1_List = (struct eccPoint **) calloc(2, sizeof(struct eccPoint*));
	toReturn -> h_0_List = (struct eccPoint **) calloc(2, sizeof(struct eccPoint*));
	toReturn -> h_1_List = (struct eccPoint **) calloc(2, sizeof(struct eccPoint*));

	toReturn -> g_0_List[0] = copyECC_Point(g_0);
	toReturn -> g_0_List[1] = copyECC_Point(g_1);

	toReturn -> g_1_List[0] = copyECC_Point(u_0);
	toReturn -> g_1_List[1] = copyECC_Point(u_1);

	toReturn -> h_0_List[0] = copyECC_Point(h_0);
	toReturn -> h_0_List[1] = copyECC_Point(h_1);

	toReturn -> h_1_List[0] = copyECC_Point(v_0);
	toReturn -> h_1_List[1] = copyECC_Point(v_1);


	return toReturn;
}


void freeTwoDH_Tuples(struct twoDH_Tuples *tuplesToFree)
{
	clearECC_Point(tuplesToFree -> g_0_List[0]);
	clearECC_Point(tuplesToFree -> g_0_List[1]);

	clearECC_Point(tuplesToFree -> g_1_List[0]);
	clearECC_Point(tuplesToFree -> g_1_List[1]);

	clearECC_Point(tuplesToFree -> h_0_List[0]);
	clearECC_Point(tuplesToFree -> h_0_List[1]);
	
	clearECC_Point(tuplesToFree -> h_1_List[0]);
	clearECC_Point(tuplesToFree -> h_1_List[1]);


	free(tuplesToFree -> g_0_List);
	free(tuplesToFree -> g_1_List);
	free(tuplesToFree -> h_0_List);
	free(tuplesToFree -> h_1_List);


	free(tuplesToFree);
}


struct twoDH_Tuples *getDH_Tuples(struct eccPoint *g_0, struct eccPoint *g_1,
								struct eccPoint *h_0, struct eccPoint *h_1,
								struct eccPoint **u_array, struct eccPoint **v_array,
								int length, struct eccParams *params, mpz_t *lambda)
{
	struct eccPoint *u_Product, *v_Product;
	struct eccPoint *u_temp, *v_temp;
	struct twoDH_Tuples *tuples;
	int i;


	u_Product = init_Identity_ECC_Point();
	v_Product = init_Identity_ECC_Point();

	for(i = 0; i < length; i ++)
	{
		u_temp = windowedScalarPoint(lambda[i], u_array[i], params);
		v_temp = windowedScalarPoint(lambda[i], v_array[i], params);

		groupOp_PlusEqual(u_Product, u_temp, params);
		groupOp_PlusEqual(v_Product, v_temp, params);

		clearECC_Point(u_temp);
		clearECC_Point(v_temp);
	}

	tuples = initTwoDH_Tuples(g_0, g_1, h_0, h_1, u_Product, v_Product);


	clearECC_Point(u_Product);
	clearECC_Point(v_Product);

	return tuples;
}


struct twoDH_Tuples *getDH_Tuples_2U(struct eccPoint *g_0, struct eccPoint *g_1,
									struct eccPoint *h_0, struct eccPoint *h_1,
									struct eccPoint **u0_array, struct eccPoint **u1_array,
									struct eccPoint **v_array,
									int length, struct eccParams *params,
									mpz_t *lambda, int lambdaIndex)
{
	struct eccPoint *u0_Product, *u1_Product, *v_Product;
	struct eccPoint *u0_temp, *u1_temp, *v_temp;
	struct twoDH_Tuples *tuples;
	int i;
	
	mpz_t tempL;
	mpz_init(tempL);


	u0_Product = init_Identity_ECC_Point();
	u1_Product = init_Identity_ECC_Point();
	v_Product = init_Identity_ECC_Point();

	for(i = 0; i < length; i ++)
	{
		mpz_set(tempL, lambda[i + lambdaIndex]);

		u0_temp = windowedScalarPoint(lambda[i + lambdaIndex], u0_array[i], params);
		u1_temp = windowedScalarPoint(lambda[i + lambdaIndex], u1_array[i], params);
		v_temp = windowedScalarPoint(lambda[i + lambdaIndex], v_array[i], params);

		groupOp_PlusEqual(u0_Product, u0_temp, params);
		groupOp_PlusEqual(u1_Product, u1_temp, params);
		groupOp_PlusEqual(v_Product, v_temp, params);

		clearECC_Point(u0_temp);
		clearECC_Point(u1_temp);
		clearECC_Point(v_temp);
	}


	tuples = initTwoDH_Tuples_2U(g_0, g_1, h_0, h_1, u0_Product, u1_Product, v_Product);

	clearECC_Point(u0_Product);
	clearECC_Point(u1_Product);
	clearECC_Point(v_Product);

	return tuples;
}


struct twoDH_Tuples *getDH_Tuples_2U_2V(struct eccPoint *g_0, struct eccPoint *g_1,
										struct eccPoint *h_0, struct eccPoint *h_1,
										struct eccPoint **u0_array, struct eccPoint **u1_array,
										struct eccPoint **v0_array, struct eccPoint **v1_array,
										int length,
										struct eccParams *params, mpz_t *lambda)
{
	struct eccPoint *u0_Product, *u1_Product, *v0_Product, *v1_Product;
	struct eccPoint *u0_temp, *u1_temp, *v0_temp, *v1_temp;
	struct twoDH_Tuples *tuples;
	int i;


	u0_Product = init_Identity_ECC_Point();
	u1_Product = init_Identity_ECC_Point();
	v0_Product = init_Identity_ECC_Point();
	v1_Product = init_Identity_ECC_Point();

	for(i = 0; i < length; i ++)
	{
		u0_temp = windowedScalarPoint(lambda[i], u0_array[i], params);
		u1_temp = windowedScalarPoint(lambda[i], u1_array[i], params);
		v0_temp = windowedScalarPoint(lambda[i], v0_array[i], params);
		v1_temp = windowedScalarPoint(lambda[i], v1_array[i], params);

		groupOp_PlusEqual(u0_Product, u0_temp, params);
		groupOp_PlusEqual(u1_Product, u1_temp, params);
		groupOp_PlusEqual(v0_Product, v0_temp, params);
		groupOp_PlusEqual(v1_Product, v1_temp, params);

		clearECC_Point(u0_temp);
		clearECC_Point(u1_temp);
		clearECC_Point(v0_temp);
		clearECC_Point(v1_temp);
	}


	tuples = initTwoDH_Tuples_2U_2V(g_0, g_1, h_0, h_1, u0_Product, u1_Product, v0_Product, v1_Product);

	clearECC_Point(u0_Product);
	clearECC_Point(u1_Product);
	clearECC_Point(v0_Product);
	clearECC_Point(v1_Product);

	return tuples;
}


void printTwoDH_Tuple(struct twoDH_Tuples *toPrint)
{
	printPoint(toPrint -> g_0_List[0]);
	printPoint(toPrint -> g_1_List[0]);
	printPoint(toPrint -> h_0_List[0]);
	printPoint(toPrint -> h_1_List[0]);
	printf("\n");

	printPoint(toPrint -> g_0_List[1]);
	printPoint(toPrint -> g_1_List[1]);
	printPoint(toPrint -> h_0_List[1]);
	printPoint(toPrint -> h_1_List[1]);
}



struct twoDH_Tuples **getAllTuplesProver(int writeSocket, int readSocket, struct params_CnC_ECC *params_P, int numInputs, int stat_SecParam,
										struct tildeCRS *receivedTildeCRS,
										gmp_randstate_t *state)
{
	struct twoDH_Tuples **tuples;
	struct timespec int_t_0, int_t_1;
	clock_t int_c_0, int_c_1;
	mpz_t *lambda;

	unsigned char *commBuffer;
	int commBufferLen = 0, bufferOffset = 0, numLambdas, i, j = 0;



	numLambdas = numInputs * stat_SecParam;
	tuples = (struct twoDH_Tuples**) calloc(numInputs, sizeof(struct twoDH_Tuples*));

	commBuffer = receiveBoth(readSocket, commBufferLen);
	lambda = deserialiseMPZ_Array(commBuffer, &bufferOffset);
	free(commBuffer);


	int_t_0 = timestamp();
	int_c_0 = clock();

	#pragma omp parallel for private(i, j) schedule(auto)
	for(i = 0; i < numInputs; i ++)
	{

		j = i * stat_SecParam;
		tuples[i] = getDH_Tuples_2U(params_P -> params -> g, params_P -> crs -> g_1,
									receivedTildeCRS -> lists[i] -> g_tilde,
									receivedTildeCRS -> lists[i] -> g_tilde,
									params_P -> crs -> h_0_List, params_P -> crs -> h_1_List,
									receivedTildeCRS -> lists[i] -> h_tildeList, stat_SecParam,
									params_P -> params, lambda, j);
	}

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "Setup Tuples");


	for(i = 0; i < numLambdas; i ++)
	{
		mpz_clear(lambda[i]);
	}

	return tuples;
}



struct twoDH_Tuples **getAllTuplesVerifier(int writeSocket, int readSocket, struct params_CnC_ECC *params_P, int numInputs, int stat_SecParam,
										struct tildeCRS *receivedTildeCRS,
										gmp_randstate_t *state)
{
	struct twoDH_Tuples **tuples;
	struct timespec int_t_0, int_t_1;
	clock_t int_c_0, int_c_1;

	mpz_t *lambda;

	unsigned char *commBuffer;
	int commBufferLen = 0, bufferOffset = 0, numLambdas, i, j = 0;


	numLambdas = numInputs * stat_SecParam;
	lambda = (mpz_t *) calloc(numLambdas, sizeof(mpz_t));
	tuples = (struct twoDH_Tuples**) calloc(numInputs, sizeof(struct twoDH_Tuples*));


	for(i = 0; i < numLambdas; i ++)
	{
		mpz_init(lambda[i]);
		mpz_urandomm(lambda[i], *state, params_P -> params -> n);
	}


	int_t_0 = timestamp();
	int_c_0 = clock();

	#pragma omp parallel for private(i, j) schedule(auto)
	for(i = 0; i < numInputs; i ++)
	{
		j = i * stat_SecParam;

		tuples[i] = getDH_Tuples_2U(params_P -> params -> g, params_P -> crs -> g_1,
									receivedTildeCRS -> lists[i] -> g_tilde,
									receivedTildeCRS -> lists[i] -> g_tilde,
									params_P -> crs -> h_0_List, params_P -> crs -> h_1_List,
									receivedTildeCRS -> lists[i] -> h_tildeList, stat_SecParam,
									params_P -> params, lambda, j);
	}

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "\nSetup Tuples");

	commBuffer = serialiseMPZ_Array(lambda, numLambdas, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);



	for(i = 0; i < numLambdas; i ++)
	{
		mpz_clear(lambda[i]);
	}
	free(lambda);


	return tuples;
}


// -----------------------------------------------------------------------------------------
// witness is basically r_j.
// g_0 is g from params, g_1 is g_1.
// h_0, h_1 are the entry from the CRSs h_0_List and h_1_List
// All u_i are equal and are g_tilde
// v_i is the i^th entry in the tildeList
void ZKPoK_Ext_DH_TupleProver(int writeSocket, int readSocket, int stat_SecParam,
							mpz_t *witness, unsigned char inputBit,
							struct eccPoint *g_0, struct eccPoint *g_1,
							struct eccPoint *h_0, struct eccPoint *h_1,
							struct eccPoint **u_array, struct eccPoint **v_array,
							struct eccParams *params, gmp_randstate_t *state,
							mpz_t *lambda, int lambdaIndex)
{
	struct twoDH_Tuples *tuples;

	mpz_t *local_Lambda = (mpz_t*) calloc(stat_SecParam, sizeof(mpz_t));
	unsigned char *J_set = (unsigned char *) calloc(2, sizeof(unsigned char)), *commBuffer;
	int i, commBufferLen = 0, bufferOffset = 0;


	for(i = 0; i < stat_SecParam; i ++)
	{
		mpz_init_set(local_Lambda[i], lambda[lambdaIndex + i]);
	}


	J_set[inputBit] = 0x01;


	tuples = getDH_Tuples(g_0, g_1, h_0, h_1, u_array, v_array,
						stat_SecParam, params, local_Lambda);

	ZKPoK_Prover_ECC_1Of2(writeSocket, readSocket, params,
						tuples -> g_0_List, tuples -> g_1_List,
						tuples -> h_0_List, tuples -> h_1_List,
						witness, J_set, state);

	for(i = 0; i < stat_SecParam; i ++)
	{
		mpz_clear(local_Lambda[i]);
	}
	free(local_Lambda);
	freeTwoDH_Tuples(tuples);
}


int ZKPoK_Ext_DH_TupleVerifier(int writeSocket, int readSocket, int stat_SecParam,
							struct eccPoint *g_0, struct eccPoint *g_1,
							struct eccPoint *h_0, struct eccPoint *h_1,
							struct eccPoint **u_array, struct eccPoint **v_array,
							struct eccParams *params, gmp_randstate_t *state,
							mpz_t *lambda, int lambdaIndex)
{
	struct twoDH_Tuples *tuples;

	unsigned char *commBuffer;
	mpz_t *local_Lambda = (mpz_t*) calloc(stat_SecParam, sizeof(mpz_t));
	int i, verified = 0, commBufferLen = 0, bufferOffset = 0;



	for(i = 0; i < stat_SecParam; i ++)
	{
		mpz_init_set(local_Lambda[i], lambda[lambdaIndex + i]);
	}

	tuples = getDH_Tuples(g_0, g_1, h_0, h_1, u_array, v_array,
						stat_SecParam, params, local_Lambda);

	verified = ZKPoK_Verifier_ECC_1Of2(writeSocket, readSocket, params,
									tuples -> g_0_List, tuples -> g_1_List,
									tuples -> h_0_List, tuples -> h_1_List, state);

	for(i = 0; i < stat_SecParam; i ++)
	{
		mpz_clear(local_Lambda[i]);
	}
	free(local_Lambda);
	freeTwoDH_Tuples(tuples);

	return verified;
}


// -----------------------------------------------------------------------------------------



void ZKPoK_Ext_DH_TupleProver_2U(int writeSocket, int readSocket, int stat_SecParam,
								mpz_t *witness, unsigned char inputBit,
								struct eccPoint *g_0, struct eccPoint *g_1,
								struct eccPoint *h_0, struct eccPoint *h_1,
								struct eccPoint **u0_array, struct eccPoint **u1_array,
								struct eccPoint **v_array, struct eccParams *params, gmp_randstate_t *state)
{
	struct twoDH_Tuples *tuples;

	mpz_t *lambda;
	unsigned char *commBuffer, *J_set = (unsigned char *) calloc(2, sizeof(unsigned char));
	int i, commBufferLen = 0, bufferOffset = 0;


	commBuffer = receiveBoth(readSocket, commBufferLen);
	lambda = deserialiseMPZ_Array(commBuffer, &bufferOffset);
	free(commBuffer);

	J_set[inputBit] = 0x01;

	tuples = getDH_Tuples_2U(g_0, g_1, h_0, h_1,
							u0_array, u1_array, v_array,
							stat_SecParam, params, lambda, 0);

	ZKPoK_Prover_ECC_1Of2(writeSocket, readSocket, params,
					tuples -> g_0_List, tuples -> g_1_List,
					tuples -> h_0_List, tuples -> h_1_List,
					witness, J_set, state);


	freeTwoDH_Tuples(tuples);
}


int ZKPoK_Ext_DH_TupleVerifier_2U(int writeSocket, int readSocket, int stat_SecParam,
								struct eccPoint *g_0, struct eccPoint *g_1,
								struct eccPoint *h_0, struct eccPoint *h_1,
								struct eccPoint **u0_array, struct eccPoint **u1_array,
								struct eccPoint **v_array,
								struct eccParams *params, gmp_randstate_t *state)
{
	struct twoDH_Tuples *tuples;

	mpz_t *lambda = (mpz_t*) calloc(stat_SecParam, sizeof(mpz_t));;
	unsigned char *commBuffer;
	int i, verified = 0, commBufferLen = 0, bufferOffset = 0;


	for(i = 0; i < stat_SecParam; i ++)
	{
		mpz_init(lambda[i]);
		mpz_urandomm(lambda[i], *state, params -> n);
	}

	commBuffer = serialiseMPZ_Array(lambda, stat_SecParam, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);

	tuples = getDH_Tuples_2U(g_0, g_1, h_0, h_1,
							u0_array, u1_array, v_array,
							stat_SecParam, params, lambda, 0);

	verified = ZKPoK_Verifier_ECC_1Of2(writeSocket, readSocket, params,
									tuples -> g_0_List, tuples -> g_1_List,
									tuples -> h_0_List, tuples -> h_1_List, state);

	freeTwoDH_Tuples(tuples);

	return verified;
}






void ZKPoK_Ext_DH_TupleProver_2U_2V(int writeSocket, int readSocket, int stat_SecParam,
								mpz_t *witness, unsigned char inputBit,
								struct eccPoint *g_0, struct eccPoint *g_1,
								struct eccPoint *h_0, struct eccPoint *h_1,
								struct eccPoint **u0_array, struct eccPoint **u1_array,
								struct eccPoint **v0_array, struct eccPoint **v1_array,
								struct eccParams *params, gmp_randstate_t *state)
{
	struct twoDH_Tuples *tuples;

	unsigned char *commBuffer, *J_set = (unsigned char *) calloc(2, sizeof(unsigned char));
	int i, commBufferLen = 0, bufferOffset = 0;
	mpz_t *lambda;


	commBuffer = receiveBoth(readSocket, commBufferLen);
	lambda = deserialiseMPZ_Array(commBuffer, &bufferOffset);
	free(commBuffer);

	tuples = getDH_Tuples_2U_2V(g_0, g_1, h_0, h_1,
								u0_array, u1_array, 
								v0_array, v1_array,
								stat_SecParam, params, lambda);
	J_set[inputBit] = 0x01;



	ZKPoK_Prover_ECC_1Of2(writeSocket, readSocket, params,
					tuples -> g_0_List, tuples -> g_1_List,
					tuples -> h_0_List, tuples -> h_1_List,
					witness, J_set, state);

	freeTwoDH_Tuples(tuples);

}



int ZKPoK_Ext_DH_TupleVerifier_2U_2V(int writeSocket, int readSocket, int stat_SecParam,
								struct eccPoint *g_0, struct eccPoint *g_1,
								struct eccPoint *h_0, struct eccPoint *h_1,
								struct eccPoint **u0_array, struct eccPoint **u1_array,
								struct eccPoint **v0_array, struct eccPoint **v1_array,
								struct eccParams *params, gmp_randstate_t *state)
{
	struct twoDH_Tuples *tuples;

	mpz_t *lambda = (mpz_t*) calloc(stat_SecParam, sizeof(mpz_t));;
	unsigned char *commBuffer;
	int i, verified = 0, commBufferLen = 0, bufferOffset = 0;


	for(i = 0; i < stat_SecParam; i ++)
	{
		mpz_init(lambda[i]);
		mpz_urandomm(lambda[i], *state, params -> n);
	}

	commBuffer = serialiseMPZ_Array(lambda, stat_SecParam, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);


	tuples = getDH_Tuples_2U_2V(g_0, g_1, h_0, h_1,
								u0_array, u1_array, 
								v0_array, v1_array,
								stat_SecParam, params, lambda);
	

	verified = ZKPoK_Verifier_ECC_1Of2(writeSocket, readSocket, params,
									tuples -> g_0_List, tuples -> g_1_List,
									tuples -> h_0_List, tuples -> h_1_List, state);

	freeTwoDH_Tuples(tuples);

	return verified;
}









void test_ZKPoK_ExtDH_Tuple_Prover(char *ipAddress)
{
	struct params_CnC_ECC *params_P;
	struct tildeList *testTildeList;
	mpz_t *witness = (mpz_t *) calloc(2, sizeof(mpz_t));

	int writeSocket, readSocket, writePort, readPort;
	struct sockaddr_in serv_addr_write;
	struct sockaddr_in serv_addr_read;
	struct eccPoint **g_tilde_list;
	struct eccPoint **h_list;


	unsigned char *commBuffer, inputBit = 0x00, *J_set;
	int bufferOffset = 0, commBufferLen = 0;
	int stat_SecParam = 8, comp_SecParam = 256;
	int i;


	readPort = 7654;
	writePort = readPort + 1;
	gmp_randstate_t *state = seedRandGen();


    set_up_client_socket(readSocket, ipAddress, readPort, serv_addr_read);
    set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr_write);


	J_set = generateJ_Set(stat_SecParam);
	params_P = setup_CnC_OT_Receiver_ECC(stat_SecParam, J_set, comp_SecParam, *state);
	commBuffer = serialiseParams_CnC_ECC(params_P, &bufferOffset);
	sendBoth(writeSocket, commBuffer, bufferOffset);
	free(commBuffer);


	for(i = 0; i < 20; i ++)
	{
		mpz_init(witness[inputBit]);
		mpz_urandomm(witness[inputBit], *state, params_P -> params -> n);
		mpz_init_set_ui(witness[1 - inputBit], 0);//witness[inputBit]);

		testTildeList = initTildeList(stat_SecParam, *witness, params_P -> crs, params_P -> params, inputBit);


		commBufferLen = 0;
		commBuffer = serialiseTildeList(testTildeList, stat_SecParam, &commBufferLen);
		sendBoth(writeSocket, commBuffer, commBufferLen);
		free(commBuffer);



		ZKPoK_Ext_DH_TupleProver_2U(writeSocket, readSocket, stat_SecParam, witness, inputBit,
									params_P -> params -> g, params_P -> crs -> g_1,
									testTildeList -> g_tilde, testTildeList -> g_tilde,
									params_P -> crs -> h_0_List, params_P -> crs -> h_1_List,
									testTildeList -> h_tildeList,
									params_P ->  params, state);
	}


	close_client_socket(readSocket);
	close_client_socket(writeSocket);

	freeParams_CnC_ECC(params_P);
	gmp_randclear(*state);
	free(state);
}



void test_ZKPoK_ExtDH_Tuple_Verifier()
{
	struct params_CnC_ECC *params_V;
	struct tildeList *receivedTildeList;
	mpz_t *tempMPZ = (mpz_t*) calloc(1, sizeof(mpz_t));

	struct sockaddr_in destWrite, destRead;
	int writeSocket, readSocket, mainWriteSock, mainReadSock;
	int writePort, readPort, i;

	unsigned char *commBuffer;

	int stat_SecParam = 8, comp_SecParam = 256;
	int cCheck = 0, verified = 0;
	int bufferOffset = 0, commBufferLen = 0;


	writePort = 7654;
	readPort = writePort + 1;
	gmp_randstate_t *state = seedRandGen();


	set_up_server_socket(destWrite, writeSocket, mainWriteSock, writePort);
	set_up_server_socket(destRead, readSocket, mainReadSock, readPort);


	commBuffer = receiveBoth(readSocket, commBufferLen);
	params_V = setup_CnC_OT_Sender_ECC(commBuffer);
	free(commBuffer);


	for(i = 0; i < 20; i ++)
	{
		bufferOffset = 0;
		commBuffer = receiveBoth(readSocket, commBufferLen);
		receivedTildeList = deserialiseTildeList(commBuffer, stat_SecParam, &bufferOffset);
		free(commBuffer);


		verified =  ZKPoK_Ext_DH_TupleVerifier_2U(writeSocket, readSocket, stat_SecParam,
												params_V -> params -> g, params_V -> crs -> g_1,
												receivedTildeList -> g_tilde, receivedTildeList -> g_tilde,
												params_V -> crs -> h_0_List, params_V -> crs -> h_1_List,
												receivedTildeList -> h_tildeList,
												params_V -> params, state);
	}

	close_server_socket(writeSocket, mainWriteSock);
	close_server_socket(readSocket, mainReadSock);


	freeParams_CnC_ECC(params_V);
	gmp_randclear(*state);
	free(state);
}








void ZKPoK_Prover_ECC_1Of2_Parallel(int writeSocket, int readSocket, int numPairs,
									struct eccParams *params,
									struct twoDH_Tuples **tuples,
									mpz_t *alphas_List,
									struct idAndValue *startOfInputChain, gmp_randstate_t *state)
{
	struct witnessStruct **witnessSet;
	mpz_t *tempMPZ;
	struct alphaAndA_Struct **alphaAndA_P;
	struct verifierCommitment_ECC **commitment_box_P;
	struct msgOneArrays_ECC **msgOne_P;
	struct idAndValue *curItem = startOfInputChain -> next;

	unsigned char *commBuffer, **listOfBuffers;
	int bufferOffset = 0, commBufferLen = 0, *arrayOfLengths;
	int i = 0;

	unsigned char **J_set = (unsigned char **) calloc(2, sizeof(unsigned char *));
	J_set[0] = (unsigned char *) calloc(2, sizeof(unsigned char));
	J_set[1] = (unsigned char *) calloc(2, sizeof(unsigned char));
	J_set[0][0] = 0x01;
	J_set[1][1] = 0x01;


	witnessSet = (struct witnessStruct **) calloc(numPairs, sizeof(struct witnessStruct *));
	alphaAndA_P = (struct alphaAndA_Struct **) calloc(numPairs, sizeof(struct alphaAndA_Struct *));
	commitment_box_P = (struct verifierCommitment_ECC **) calloc(numPairs, sizeof(struct verifierCommitment_ECC *));
	msgOne_P = (struct msgOneArrays_ECC **) calloc(numPairs, sizeof(struct msgOneArrays_ECC));


	listOfBuffers = (unsigned char **) calloc(numPairs, sizeof(unsigned char*));
	arrayOfLengths = (int*) calloc(numPairs, sizeof(int));


	#pragma omp parallel for private(i) schedule(auto)
	for(i = 0; i < numPairs; i ++)
	{
		witnessSet[i] = proverSetupWitnesses_1(alphas_List + i);
	}


	bufferOffset = 0;
	commBufferLen = 0;
	for(i = 0; i < numPairs; i ++)
	{
		alphaAndA_P[i] = proverSetupCommitment_ECC_1Of2(params, *state);
		commBufferLen += sizeOfSerial_ECCPoint(alphaAndA_P[i] -> alpha);
	}
	commBuffer = (unsigned char *) calloc(commBufferLen, sizeof(unsigned char));
	for(i = 0; i < numPairs; i ++)
	{
		serialise_ECC_Point(alphaAndA_P[i] -> alpha, commBuffer, &bufferOffset);
	}
	sendBoth(writeSocket, commBuffer, bufferOffset);
	free(commBuffer);
	// Round 1


	bufferOffset = 0;
	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	for(i = 0; i < numPairs; i ++)
	{
		commitment_box_P[i] = initVerifierCommitment_ECC();
		commitment_box_P[i] -> C_commit = deserialise_ECC_Point(commBuffer, &bufferOffset);
		// Round 2
	}
	free(commBuffer);

	for(i = 0; i < numPairs; i ++)
	{
		bufferOffset = 0;
		msgOne_P[i] = proverMessageOne_ECC_1Of2(params, J_set[curItem -> value], alphaAndA_P[i] -> alpha, 
										tuples[i] -> g_0_List, tuples[i] -> g_1_List,
										tuples[i] -> h_0_List, tuples[i] -> h_1_List, *state);
		curItem = curItem -> next;
	}
	commBuffer = serialise_Array_A_B_Arrays_ECC(msgOne_P, numPairs, 2, &bufferOffset);
	sendBoth(writeSocket, commBuffer, bufferOffset);
	free(commBuffer);
	// Round 3

	bufferOffset = 0;
	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	for(i = 0; i < numPairs; i ++)
	{
		deserialisedSecrets_CommitmentBox(commitment_box_P[i], commBuffer, &bufferOffset);
	}
	free(commBuffer);
	// Round 4


	commBufferLen = 0;
	curItem = startOfInputChain -> next;
	for(i = 0; i < numPairs; i ++)
	{
		listOfBuffers[i] = proverMessageTwo_ECC_1Of2(params, J_set[curItem -> value], commitment_box_P[i], msgOne_P[i], witnessSet[i], alphaAndA_P[i], arrayOfLengths + i);
		commBufferLen += arrayOfLengths[i];
		curItem = curItem -> next;
	}

	bufferOffset = 0;
	commBuffer = (unsigned char *) calloc(commBufferLen, sizeof(unsigned char));
	for(i = 0; i < numPairs; i ++)
	{
		memcpy(commBuffer + bufferOffset, listOfBuffers[i], arrayOfLengths[i]);
		bufferOffset += arrayOfLengths[i];
		free(listOfBuffers[i]);
	}
	sendBoth(writeSocket, commBuffer, bufferOffset);
	free(commBuffer);
	// Round 5
}



int ZKPoK_Verifier_ECC_1Of2_Parallel(int writeSocket, int readSocket, int numPairs, struct eccParams *params,
						struct twoDH_Tuples **tuples, gmp_randstate_t *state)
{
	struct alphaAndA_Struct **alphaAndA_V;
	struct verifierCommitment_ECC **commitment_box_V;
	struct msgOneArrays_ECC **msgOne_V;
	mpz_t *tempMPZ, **Z_array_V, **cShares_V;
	unsigned char *commBuffer, **listOfBuffers;
	int *arrayOfLengths;


	int cCheck = 0, verified = 0, bufferOffset = 0, commBufferLen = 0, i, j;


	alphaAndA_V = (struct alphaAndA_Struct **) calloc(numPairs, sizeof(struct alphaAndA_Struct *));
	commitment_box_V = (struct verifierCommitment_ECC **) calloc(numPairs, sizeof(struct verifierCommitment_ECC *));
	msgOne_V = (struct msgOneArrays_ECC **) calloc(numPairs, sizeof(struct msgOneArrays_ECC));


	Z_array_V = (mpz_t**) calloc(numPairs, sizeof(mpz_t*));
	cShares_V = (mpz_t**) calloc(numPairs, sizeof(mpz_t*));
	listOfBuffers = (unsigned char **) calloc(numPairs, sizeof(unsigned char*));
	arrayOfLengths = (int*) calloc(numPairs, sizeof(int));



	bufferOffset = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	for(i = 0; i < numPairs; i ++)
	{
		alphaAndA_V[i] = (struct alphaAndA_Struct *) calloc(1, sizeof(struct alphaAndA_Struct));
		alphaAndA_V[i] -> alpha = deserialise_ECC_Point(commBuffer, &bufferOffset);
	}
	free(commBuffer);
	// Round 1


	commBufferLen = 0;
	for(i = 0; i < numPairs; i ++)
	{
		commitment_box_V[i] = verifierSetupCommitment_ECC_1Of2(params, alphaAndA_V[i] -> alpha, *state);
		listOfBuffers[i] = (unsigned char *) calloc(sizeOfSerial_ECCPoint(commitment_box_V[i] -> C_commit), sizeof(unsigned char));
		serialise_ECC_Point(commitment_box_V[i] -> C_commit, listOfBuffers[i], arrayOfLengths + i);
		commBufferLen += arrayOfLengths[i];
	}

	bufferOffset = 0;
	commBuffer = (unsigned char *) calloc(commBufferLen, sizeof(unsigned char));
	for(i = 0; i < numPairs; i ++)
	{
		memcpy(commBuffer + bufferOffset, listOfBuffers[i], arrayOfLengths[i]);
		bufferOffset += arrayOfLengths[i];
		free(listOfBuffers[i]);
	}
	sendBoth(writeSocket, commBuffer, bufferOffset);
	free(commBuffer);
	// Round 2


	bufferOffset = 0;
	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	for(i = 0; i < numPairs; i ++)
	{
		msgOne_V[i] = initMsgOneArray_ECC(2);
		for(j = 0; j < 2; j ++)
		{
			msgOne_V[i] -> A_array[j] = deserialise_ECC_Point(commBuffer, &bufferOffset);
			msgOne_V[i] -> B_array[j] = deserialise_ECC_Point(commBuffer, &bufferOffset);
		}
	}
	free(commBuffer);
	// Round 3


	bufferOffset = 0;
	commBufferLen = 0;
	for(i = 0; i < numPairs; i ++)
	{
		arrayOfLengths[i] = 0;
		listOfBuffers[i] = verifierQuery_ECC_1Of2(commitment_box_V[i], arrayOfLengths + i);
		commBufferLen += arrayOfLengths[i];
	}
	commBuffer = (unsigned char *) calloc(commBufferLen, sizeof(unsigned char));
	for(i = 0; i < numPairs; i ++)
	{
		memcpy(commBuffer + bufferOffset, listOfBuffers[i], arrayOfLengths[i]);
		bufferOffset += arrayOfLengths[i];
		free(listOfBuffers[i]);
	}
	sendBoth(writeSocket, commBuffer, bufferOffset);
	free(commBuffer);
	// Round 4


	bufferOffset = 0;
	commBufferLen = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);

	for(i = 0; i < numPairs; i ++)
	{
		Z_array_V[i] = deserialiseMPZ_Array(commBuffer, &bufferOffset);
		cShares_V[i] = deserialiseMPZ_Array(commBuffer, &bufferOffset);
		tempMPZ = deserialiseMPZ(commBuffer, &bufferOffset);
		mpz_set(alphaAndA_V[i] -> a, *tempMPZ);
	}
	free(commBuffer);


	#pragma omp parallel for private(i) schedule(auto)
	for(i = 0; i < numPairs; i ++)
	{
		verified |= verifierChecks_ECC_1Of2(params,
											tuples[i] -> g_0_List, tuples[i] -> g_1_List,
											tuples[i] -> h_0_List, tuples[i] -> h_1_List,
											Z_array_V[i],
											msgOne_V[i] -> A_array, msgOne_V[i] -> B_array,
											alphaAndA_V[i], cShares_V[i], commitment_box_V[i] -> c);
	}


	for(i = 0; i < numPairs; i ++)
	{
		for(j = 0; j < 2; j ++)
		{
			mpz_clear(Z_array_V[i][j]);
			mpz_clear(cShares_V[i][j]);
		}
		free(Z_array_V[i]);
		free(cShares_V[i]);
	}

	mpz_clear(*tempMPZ);
	free(Z_array_V);
	free(cShares_V);
	free(tempMPZ);

	return verified;
}