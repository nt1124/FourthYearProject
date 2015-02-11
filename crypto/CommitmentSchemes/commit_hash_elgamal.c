struct elgamal_commit_key *deserialise_hash_elgamal_Kbox(unsigned char **xBytes, int *xBytesLen, unsigned char *inputBuffer, int *bufferOffset)
{
	struct elgamal_commit_key *k = init_commit_key();
	mpz_t *temp = (mpz_t*) calloc(1, sizeof(mpz_t));
	unsigned char *hashedX;
	int curLength;

	mpz_init(*temp);


	memcpy(&curLength, inputBuffer + *bufferOffset, sizeof(int));
	*bufferOffset += sizeof(int);
	convertBytesToMPZ(temp, inputBuffer + *bufferOffset, curLength);
	*bufferOffset += curLength;
	mpz_set(k -> r, *temp);


	// Get length of xBytes
	memcpy(xBytesLen, inputBuffer + *bufferOffset, sizeof(int));
	*bufferOffset += sizeof(int);

	// Hash X and use that for k -> x. Means we verify that H(x) was the value the commiter committed to.
	// However we still pass back xBytes as that's actually the value that was committed to
	*xBytes = (unsigned char*) calloc(*xBytesLen, sizeof(unsigned char));
	memcpy(*xBytes, inputBuffer + *bufferOffset, *xBytesLen);
	hashedX = sha_256_hash(*xBytes, *xBytesLen);

	convertBytesToMPZ(temp, hashedX, 32);
	*bufferOffset += *xBytesLen;
	mpz_set(k -> x, *temp);



	return k;
}


void create_commit_hash_box_key(struct commit_batch_params *params, unsigned char *toCommit, int toCommitLen, gmp_randstate_t state,
						struct elgamal_commit_box *c, struct elgamal_commit_key *k)
{
	unsigned char *hashedValue = sha_256_hash(toCommit, toCommitLen);

	mpz_t r, *x = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_init(*x);
	mpz_init(r);


	convertBytesToMPZ(x, hashedValue, 32);

	mpz_urandomm(r, state, params -> group -> p);

	mpz_powm(c -> u, params -> group -> g, r, params -> group -> p);

	mpz_powm(c -> v, params -> h, r, params -> group -> p);
	mpz_mul(c -> v, c -> v, *x);
	mpz_mod(c -> v, c -> v, params -> group -> p);


	convertBytesToMPZ(x, toCommit, toCommitLen);
	mpz_set(k -> r, r);
	mpz_set(k -> x, *x);


	free(x);
}


struct elgamal_commit_key *commit_hash_elgamal_C(struct commit_batch_params *params,
												unsigned char *toCommit, int toCommitLen,
												unsigned char *outputBuffer, int *bufferOffset,
												gmp_randstate_t state)
{
	struct elgamal_commit_box *c = init_commit_box();
	struct elgamal_commit_key *k = init_commit_key();

	create_commit_hash_box_key(params, toCommit, toCommitLen, state, c, k);

	serialise_elgamal_Cbox(c, outputBuffer, bufferOffset);

	return k;
}


struct elgamal_commit_box *commit_hash_elgamal_R(struct commit_batch_params *params, unsigned char *inputBuffer, int *bufferOffset)
{
	struct elgamal_commit_box *c = deserialise_elgamal_Cbox(inputBuffer, bufferOffset);

	return c;
}

void decommit_hash_elgamal_C(struct commit_batch_params *params, struct elgamal_commit_key *k, unsigned char *outputBuffer, int *bufferOffset)
{
	serialise_elgamal_Kbox(k, outputBuffer, bufferOffset);
}


unsigned char *decommit_hash_elgamal_R(struct commit_batch_params *params, struct elgamal_commit_box *c,
										unsigned char *keyBuffer, int *keyOffset, int *outputLength)
{
	unsigned char **xBytes = (unsigned char**) calloc(1, sizeof(unsigned char*));
	int xBytesLen = 0;

	struct elgamal_commit_key *k = deserialise_hash_elgamal_Kbox(xBytes, &xBytesLen, keyBuffer, keyOffset);
	unsigned char *output;
	mpz_t u_test, v_test;

	mpz_init(u_test);
	mpz_init(v_test);

	if(0 > mpz_cmp(params -> group -> p, params -> h))
	{
		printf("111a\n");
		*outputLength = 0;
		return NULL;
	}

	mpz_powm(u_test, params -> group -> g, k -> r, params -> group -> p);

	mpz_powm(v_test, params -> h, k -> r, params -> group -> p);
	mpz_mul(v_test, v_test, k -> x);
	mpz_mod(v_test, v_test, params -> group -> p);


	if( 0 > mpz_cmp(params -> group -> p, k -> x) || 0 != mpz_cmp(c -> u, u_test) || 0 != mpz_cmp(c -> v, v_test) )
	{
		printf("111b\n");
		*outputLength = 0;
		return NULL;
	}



	*outputLength = xBytesLen;

	return *xBytes;
}


struct commit_batch_params *setup_hash_elgamal_C(int writeSocket, int readSocket, int securityParam, gmp_randstate_t state)
{	
	struct commit_batch_params *params = setup_elgamal_C(writeSocket, readSocket, securityParam, state);

	return params;
}


struct commit_batch_params *setup_hash_elgamal_R(int writeSocket, int readSocket, int securityParam, gmp_randstate_t state)
{
	struct commit_batch_params *params = setup_elgamal_R(writeSocket, readSocket);

	return params;
}