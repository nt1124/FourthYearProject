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
										int length,
										struct eccParams *params, mpz_t *lambda)
{
	struct eccPoint *u0_Product, *u1_Product, *v_Product;
	struct eccPoint *u0_temp, *u1_temp, *v_temp;
	struct twoDH_Tuples *tuples;
	int i;


	u0_Product = init_Identity_ECC_Point();
	u1_Product = init_Identity_ECC_Point();
	v_Product = init_Identity_ECC_Point();

	for(i = 0; i < length; i ++)
	{
		u0_temp = windowedScalarPoint(lambda[i], u0_array[i], params);
		u1_temp = windowedScalarPoint(lambda[i], u1_array[i], params);
		v_temp = windowedScalarPoint(lambda[i], v_array[i], params);

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



void printTwoDH_TuplePowered(struct twoDH_Tuples *toPrint, mpz_t power, struct eccParams *params)
{
	struct eccPoint *temp;

	temp = windowedScalarPoint(power, toPrint -> g_0_List[0], params);
	printPoint(temp);
	temp = windowedScalarPoint(power, toPrint -> g_1_List[0], params);
	printPoint(temp);
	printf("\n");

	printPoint(toPrint -> h_0_List[0]);
	printPoint(toPrint -> h_1_List[0]);
	printf("\n\n");


	temp = windowedScalarPoint(power, toPrint -> g_0_List[1], params);
	printPoint(temp);
	temp = windowedScalarPoint(power, toPrint -> g_1_List[1], params);
	printPoint(temp);
	printf("\n");

	printPoint(toPrint -> h_0_List[1]);
	printPoint(toPrint -> h_1_List[1]);
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

	/*
	commBuffer = receiveBoth(readSocket, commBufferLen);
	local_Lambda = deserialiseMPZ_Array(commBuffer, &bufferOffset);
	free(commBuffer);
	*/

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
		// mpz_init(local_Lambda[i]);
		// mpz_urandomm(local_Lambda[i], *state, params -> n);
		mpz_init_set(local_Lambda[i], lambda[lambdaIndex + i]);
	}

	/*
	commBuffer = serialiseMPZ_Array(local_Lambda, stat_SecParam, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);
	*/

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
	int i, verified = 0, commBufferLen = 0, bufferOffset = 0;


	commBuffer = receiveBoth(readSocket, commBufferLen);
	lambda = deserialiseMPZ_Array(commBuffer, &bufferOffset);
	free(commBuffer);

	J_set[inputBit] = 0x01;

	tuples = getDH_Tuples_2U(g_0, g_1, h_0, h_1,
							u0_array, u1_array, v_array,
							stat_SecParam, params, lambda);

	// printTwoDH_TuplePowered(tuples, *witness, params);

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
							stat_SecParam, params, lambda);

	verified = ZKPoK_Verifier_ECC_1Of2(writeSocket, readSocket, params,
									tuples -> g_0_List, tuples -> g_1_List,
									tuples -> h_0_List, tuples -> h_1_List, state);

	freeTwoDH_Tuples(tuples);
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
	int i, verified = 0, commBufferLen = 0, bufferOffset = 0;
	mpz_t *lambda;


	commBuffer = receiveBoth(readSocket, commBufferLen);
	lambda = deserialiseMPZ_Array(commBuffer, &bufferOffset);
	free(commBuffer);

	tuples = getDH_Tuples_2U_2V(g_0, g_1, h_0, h_1,
								u0_array, u1_array, 
								v0_array, v1_array,
								stat_SecParam, params, lambda);
	J_set[inputBit] = 0x01;


	printTwoDH_TuplePowered(tuples, *witness, params);

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


	unsigned char *commBuffer, inputBit = 0x00;
	int bufferOffset = 0, commBufferLen = 0;
	int stat_SecParam = 8, comp_SecParam = 256;


	readPort = 7654;
	writePort = readPort + 1;
	gmp_randstate_t *state = seedRandGen();


    set_up_client_socket(readSocket, ipAddress, readPort, serv_addr_read);
    set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr_write);


	params_P = setup_CnC_OT_Receiver_ECC(stat_SecParam, comp_SecParam, *state);
	commBuffer = serialiseParams_CnC_ECC(params_P, &bufferOffset);
	sendBoth(writeSocket, commBuffer, bufferOffset);
	free(commBuffer);


	mpz_init(witness[inputBit]);
	mpz_urandomm(witness[inputBit], *state, params_P -> params -> n);
	mpz_init_set(witness[1 - inputBit], witness[inputBit]);

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


	close_server_socket(writeSocket, mainWriteSock);
	close_server_socket(readSocket, mainReadSock);

	printf("%d\n",verified);
	fflush(stdout);

	freeParams_CnC_ECC(params_V);
	gmp_randclear(*state);
	free(state);
}