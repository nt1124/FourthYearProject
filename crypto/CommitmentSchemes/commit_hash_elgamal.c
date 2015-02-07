struct elgamal_commit_key *deserialise_hash_elgamal_Kbox(unsigned char *xBytes, int *xBytesLen, unsigned char *inputBuffer, int *bufferOffset)
{
	struct elgamal_commit_key *k = init_commit_key();
	mpz_t *temp = (mpz_t*) calloc(1, sizeof(mpz_t));
	unsigned char *hashedX;
	int curLength;

	mpz_init(*temp);


	memcpy(xBytesLen, inputBuffer + *bufferOffset, sizeof(int));
	*bufferOffset += sizeof(int);
	xBytes = (unsigned char*) calloc(*xBytesLen, sizeof(unsigned char));
	memcpy(xBytes, inputBuffer + *bufferOffset, *xBytesLen);

	hashedX = SHA_512_Hash(xBytes, xBytesLen);
	convertBytesToMPZ(temp, hashedX, 512 / 8);
	*bufferOffset += *xBytesLen;
	mpz_set(k -> r, *temp);


	memcpy(&curLength, inputBuffer + *bufferOffset, sizeof(int));
	*bufferOffset += sizeof(int);
	convertBytesToMPZ(temp, inputBuffer + *bufferOffset, curLength);
	*bufferOffset += curLength;
	mpz_set(k -> x, *temp);

	return k;
}



struct elgamal_commit_key *commit_hash_elgamal_C(struct commit_batch_params *params,
												unsigned char *toCommit, int toCommitLen,
												unsigned char *outputBuffer, int *bufferOffset,
												gmp_randstate_t state)
{
	string toCommitAsStr(toCommit)
	unsigned char *hashedValue = SHA_512_Hash(toCommitAsStr, toCommitLen);

	struct elgamal_commit_key *k = single_commit_elgamal_C(params, hashedValue, 512 / 8, outputBuffer, bufferOffset, state);
	mpz_set(k -> x, toCommit);

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
	struct elgamal_commit_key *k = deserialise_elgamal_Kbox(keyBuffer, keyOffset);
	unsigned char *output;
	mpz_t u_test, v_test, tempX, hashedX;

	mpz_init(u_test);
	mpz_init(v_test);
	mpz_init(tempX);
	mpz_init(hashedX);

	mpz_set(tempX, k -> x);
	mpz_set(k -> x, hashedX);


	output = 

	if( )

	return convertMPZToBytes(k -> x, outputLength);
}
/*
unsigned char *single_decommit_elgamal_R(struct commit_batch_params *params, struct elgamal_commit_box *c, unsigned char *keyBuffer, int *keyOffset, int *outputLength)
{
	struct elgamal_commit_key *k = deserialise_elgamal_Kbox(keyBuffer, keyOffset);
	unsigned char *output;
	mpz_t u_test, v_test;

	mpz_init(u_test);
	mpz_init(v_test);

	if(0 > mpz_cmp(params -> group -> p, params -> h))
	{
		return NULL;
	}

	mpz_powm(u_test, params -> group -> g, k -> r, params -> group -> p);

	mpz_powm(v_test, params -> h, k -> r, params -> group -> p);
	mpz_mul(v_test, v_test, k -> x);
	mpz_mod(v_test, v_test, params -> group -> p);


	if( 0 > mpz_cmp(params -> group -> p, k -> x) || 0 != mpz_cmp(c -> u, u_test) || 0 != mpz_cmp(c -> v, v_test) )
	{
		return NULL;
	}

	return convertMPZToBytes(k -> x, outputLength);
}
*/

struct commit_batch_params *setup_hash_elgamal_C(int writeSocket, int readSocket, int securityParam, gmp_randstate_t state)
{	
	struct commit_batch_params *params = setup_elgamal_C(writeSocket, readSocket, securityParam, state)

	

	return group;
}


struct commit_batch_params *setup_hash_elgamal_R(int writeSocket, int readSocket, int securityParam, gmp_randstate_t state)
{
	struct commit_batch_params *params = setup_elgamal_R(writeSocket, readSocket)

	return params;
}