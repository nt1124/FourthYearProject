struct commit_batch_params *init_commit_batch_params()
{
	struct commit_batch_params *toReturn = (struct commit_batch_params*) calloc(1, sizeof(struct commit_batch_params));

	return toReturn;
}


struct commit_batch_params *generate_commit_params(int securityParam, gmp_randstate_t state)
{
	struct commit_batch_params *toReturn = init_commit_batch_params();
	mpz_t a;
	mpz_init(a);

	toReturn -> params = initBrainpool_256_Curve();

	mpz_urandomm(a, state, toReturn -> params -> n);
	toReturn -> h = windowedScalarPoint(a, toReturn -> params -> g, toReturn -> params);

	return toReturn;
}


struct elgamal_commit_box *init_commit_box()
{
	struct elgamal_commit_box *c = (struct elgamal_commit_box*) calloc(1, sizeof(struct elgamal_commit_box));

	return c;
}


struct elgamal_commit_key *init_commit_key()
{
	struct elgamal_commit_key *k = (struct elgamal_commit_key*) calloc(1, sizeof(struct elgamal_commit_key));

	return k;
}


void create_commit_box_key(struct commit_batch_params *params, unsigned char *toCommit, int toCommitLen, gmp_randstate_t state,
						struct elgamal_commit_box *c, struct elgamal_commit_key *k)
{
	struct eccPoint *g_x;
	mpz_t r, *x = (mpz_t*) calloc(1, sizeof(mpz_t));


	mpz_init(r);
	mpz_init(*x);

	convertBytesToMPZ(x, toCommit, toCommitLen);
	g_x = windowedScalarPoint(*x, params -> params -> g, params -> params);

	mpz_urandomm(r, state, params -> params -> n);

	c -> u = windowedScalarPoint(r, params -> params -> g, params -> params);
	c -> v = windowedScalarPoint(r, params -> h, params -> params);
	groupOp_PlusEqual(c -> v, g_x, params -> params);

	mpz_set(k -> r, r);
	k -> x = toCommit;

	free(x);
}


struct elgamal_commit_key *single_commit_elgamal_C(struct commit_batch_params *params,
												unsigned char *toCommit, int toCommitLen,
												unsigned char *outputBuffer, int *bufferOffset,
												gmp_randstate_t state)
{
	struct elgamal_commit_box *c = init_commit_box();
	struct elgamal_commit_key *k = init_commit_key();

	create_commit_box_key(params, toCommit, toCommitLen, state, c, k);


	return k;
}


struct elgamal_commit_box *single_commit_elgamal_R(struct commit_batch_params *params, unsigned char *inputBuffer, int *bufferOffset)
{
	struct elgamal_commit_box *c = deserialise_elgamal_Cbox(inputBuffer, bufferOffset);

	return c;
}


unsigned char *single_decommit_elgamal_R(struct commit_batch_params *params, struct elgamal_commit_box *c, struct elgamal_commit_key *k, int msgLength, int *outputLength)
{
	struct eccPoint *u_test, *v_test, *g_x;
	int validPointTest = 0;
	mpz_t *x = (mpz_t*) calloc(1, sizeof(mpz_t));


	mpz_init(*x);

	convertBytesToMPZ(x, k -> x, msgLength);
	g_x = windowedScalarPoint(*x, params -> params -> g, params -> params);


	u_test = windowedScalarPoint(k -> r, params -> params -> g, params -> params);
	v_test = windowedScalarPoint(k -> r, params -> h, params -> params);
	groupOp_PlusEqual(v_test, g_x, params -> params);

	// validPointTest |= checkValidECC_Point(params -> h, params -> params);
	// validPointTest |= checkValidECC_Point(g_x, params -> params);
	// validPointTest |= checkValidECC_Point(v_test, params -> params);
	
	if(0 != validPointTest)
	{
		return NULL;
	}


	return k -> x;
}





int send_commit_batch_params(int writeSocket, int readSocket, struct commit_batch_params *params)
{
	unsigned char *curBytes, *paramsBytes, *hBytes;
	int curLength, paramsLength, hLength, tempOffset = 0;


	paramsBytes = serialiseECC_Params(params -> params, &paramsLength);

	hLength = sizeOfSerial_ECCPoint(params -> h);
	hBytes = (unsigned char *) calloc(hLength, sizeof(unsigned char));
	serialise_ECC_Point(params -> h, hBytes, &tempOffset);

	curLength = paramsLength + hLength;
	curBytes = (unsigned char*) calloc(curLength, sizeof(unsigned char));

	tempOffset = 0;
	memcpy(curBytes + tempOffset, paramsBytes, paramsLength);
	tempOffset += paramsLength;

	memcpy(curBytes + tempOffset, hBytes, hLength);
	tempOffset += hLength;


	sendBoth(writeSocket, (octet*) curBytes, curLength);

	free(curBytes);
	free(paramsBytes);
	free(hBytes);

	return 1;
}


struct commit_batch_params *receive_commit_batch_params(int writeSocket, int readSocket)
{
	struct commit_batch_params *params = init_commit_batch_params();
	int curLength, tempIndex = 0;
	unsigned char *curBytes;


	curBytes = receiveBoth(readSocket, curLength);

	params -> params = deserialiseECC_Params(curBytes, &tempIndex);
	params -> h = deserialise_ECC_Point(curBytes, &tempIndex);

	free(curBytes);

	return params;
}


struct commit_batch_params *setup_elgamal_C(int writeSocket, int readSocket, int securityParam, gmp_randstate_t state)
{
	struct commit_batch_params *params = generate_commit_params(securityParam, state);

	send_commit_batch_params(writeSocket, readSocket, params);

	return params;
}


struct commit_batch_params *setup_elgamal_R(int writeSocket, int readSocket)
{
	struct commit_batch_params *params = receive_commit_batch_params(writeSocket, readSocket);

	return params;
}
