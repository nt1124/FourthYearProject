struct commit_batch_params *init_commit_batch_params()
{
	struct commit_batch_params *toReturn = (struct commit_batch_params*) calloc(1, sizeof(struct commit_batch_params));

	// toReturn -> group = initGroupStruct();

	mpz_init(toReturn -> h);

	return toReturn;
}


struct commit_batch_params *generate_commit_params(int securityParam, gmp_randstate_t state)
{
	struct commit_batch_params *toReturn = init_commit_batch_params();
	mpz_t a;
	mpz_init(a);

	toReturn -> group = generateGroup(securityParam, state);

	mpz_urandomm(a, state, toReturn -> group -> p);
	mpz_powm(toReturn -> h, toReturn -> group -> g, a, toReturn -> group -> p);

	return toReturn;
}


int getSerialSizeElgamal(struct DDH_Group *group, int numToSerialise, int numInEach)
{
	int sizeOfP = mpz_sizeinbase(group -> p, 2);

	if(0 == sizeOfP % 8)
	{
		sizeOfP = sizeOfP / 8;
	}
	else
	{
		sizeOfP = (sizeOfP / 8) + 1;
	}

	sizeOfP *= (numInEach * numToSerialise);

	return sizeOfP;
}


struct elgamal_commit_box *init_commit_box()
{
	struct elgamal_commit_box *c = (struct elgamal_commit_box*) calloc(1, sizeof(struct elgamal_commit_box));

	mpz_init(c -> u);
	mpz_init(c -> v);

	return c;
}


struct elgamal_commit_key *init_commit_key()
{
	struct elgamal_commit_key *k = (struct elgamal_commit_key*) calloc(1, sizeof(struct elgamal_commit_key));

	mpz_init(k -> r);
	mpz_init(k -> x);

	return k;
}


void serialise_elgamal_Cbox(struct elgamal_commit_box *c, unsigned char *outputBuffer, int *bufferOffset)
{
	unsigned char *curBytes;
	int curLength;

	curBytes = convertMPZToBytes(c -> u, &curLength);
	memcpy(outputBuffer + *bufferOffset, &curLength, sizeof(int));
	*bufferOffset += sizeof(int);
	memcpy(outputBuffer + *bufferOffset, curBytes, curLength);
	*bufferOffset += curLength;

	curBytes = convertMPZToBytes(c -> v, &curLength);
	memcpy(outputBuffer + *bufferOffset, &curLength, sizeof(int));
	*bufferOffset += sizeof(int);
	memcpy(outputBuffer + *bufferOffset, curBytes, curLength);
	*bufferOffset += curLength;
}


struct elgamal_commit_box *deserialise_elgamal_Cbox(unsigned char *inputBuffer, int *bufferOffset)
{
	struct elgamal_commit_box *c = init_commit_box();
	mpz_t *temp = (mpz_t*) calloc(1, sizeof(mpz_t));
	int curLength;

	mpz_init(*temp);


	memcpy(&curLength, inputBuffer + *bufferOffset, sizeof(int));
	*bufferOffset += sizeof(int);
	convertBytesToMPZ(temp, inputBuffer + *bufferOffset, curLength);
	*bufferOffset += curLength;
	mpz_set(c -> u, *temp);

	memcpy(&curLength, inputBuffer + *bufferOffset, sizeof(int));
	*bufferOffset += sizeof(int);
	convertBytesToMPZ(temp, inputBuffer + *bufferOffset, curLength);
	*bufferOffset += curLength;
	mpz_set(c -> v, *temp);

	return c;
}


void serialise_elgamal_Kbox(struct elgamal_commit_key *k, unsigned char *outputBuffer, int *bufferOffset)
{
	unsigned char *curBytes;
	int curLength;

	curBytes = convertMPZToBytes(k -> r, &curLength);
	memcpy(outputBuffer + *bufferOffset, &curLength, sizeof(int));
	*bufferOffset += sizeof(int);
	memcpy(outputBuffer + *bufferOffset, curBytes, curLength);
	*bufferOffset += curLength;

	curBytes = convertMPZToBytes(k -> x, &curLength);
	memcpy(outputBuffer + *bufferOffset, &curLength, sizeof(int));
	*bufferOffset += sizeof(int);
	memcpy(outputBuffer + *bufferOffset, curBytes, curLength);
	*bufferOffset += curLength;
}


struct elgamal_commit_key *deserialise_elgamal_Kbox(unsigned char *inputBuffer, int *bufferOffset)
{
	struct elgamal_commit_key *k = init_commit_key();
	mpz_t *temp = (mpz_t*) calloc(1, sizeof(mpz_t));
	int curLength;

	mpz_init(*temp);


	memcpy(&curLength, inputBuffer + *bufferOffset, sizeof(int));
	*bufferOffset += sizeof(int);
	convertBytesToMPZ(temp, inputBuffer + *bufferOffset, curLength);
	*bufferOffset += curLength;
	mpz_set(k -> r, *temp);

	memcpy(&curLength, inputBuffer + *bufferOffset, sizeof(int));
	*bufferOffset += sizeof(int);
	convertBytesToMPZ(temp, inputBuffer + *bufferOffset, curLength);
	*bufferOffset += curLength;
	mpz_set(k -> x, *temp);

	return k;
}


struct elgamal_commit_key *single_commit_elgamal_C(struct commit_batch_params *params, mpz_t valueToCommit,
												unsigned char *outputBuffer, int *bufferOffset,
												gmp_randstate_t state)
{
	struct elgamal_commit_box *c = init_commit_box();
	struct elgamal_commit_key *k = init_commit_key();

	mpz_t r;
	mpz_init(r);


	mpz_urandomm(r, state, params -> group -> p);

	mpz_powm(c -> u, params -> group -> g, r, params -> group -> p);

	mpz_powm(c -> v, params -> h, r, params -> group -> p);
	mpz_mul(c -> v, c -> v, valueToCommit);
	mpz_mod(c -> v, c -> v, params -> group -> p);

	serialise_elgamal_Cbox(c, outputBuffer, bufferOffset);

	mpz_set(k -> r, r);
	mpz_set(k -> x, valueToCommit);

	return k;
}


// Actually in retrospect we don't need to do this, just store the string...
struct elgamal_commit_box *single_commit_elgamal_R(struct commit_batch_params *params, unsigned char *inputBuffer, int *bufferOffset)
{
	struct elgamal_commit_box *c = deserialise_elgamal_Cbox(inputBuffer, bufferOffset);

	return c;
}


// Very simple, just serialises the k.
void single_commit_elgamal_C(struct commit_batch_params *params, struct elgamal_commit_key *k, unsigned char *outputBuffer, int *bufferOffset)
{
	serialise_elgamal_Kbox(k, outputBuffer, bufferOffset);
}


unsigned char single_decommit_elgamal_R(struct commit_batch_params *params, struct elgamal_commit_box *c, unsigned char *keyBuffer, int *keyOffset)
{
	struct elgamal_commit_key *k = deserialise_elgamal_Kbox(keyBuffer, keyOffset);
	mpz_t u_test, v_test;

	mpz_init(u_test);
	mpz_init(v_test);

	if(0 > mpz_cmp(params -> group -> p, params -> h))
	{
		return 0;
	}

	mpz_powm(u_test, params -> group -> g, k -> r, params -> group -> p);

	mpz_powm(v_test, params -> h, k -> r, params -> group -> p);
	mpz_mul(v_test, v_test, k -> x);
	mpz_mod(v_test, v_test, params -> group -> p);


	if( 0 > mpz_cmp(params -> group -> p, k -> x) || 0 != mpz_cmp(c -> u, u_test) || 0 != mpz_cmp(c -> v, v_test) )
	{
		return 0;
	}

	return 1;
}


int send_commit_batch_params(int writeSocket, int readSocket, struct commit_batch_params *params)
{
	unsigned char *curBytes, *pBytes, *gBytes, *hBytes;
	int curLength, pLength, gLength, hLength, index;


	pBytes = convertMPZToBytes( params -> group -> p, &pLength);
	gBytes = convertMPZToBytes( params -> group -> g, &gLength);
	hBytes = convertMPZToBytes( params -> h, &hLength);

	curLength = pLength + gLength + hLength;
	curBytes = (unsigned char*) calloc(3 * sizeof(int) + curLength, sizeof(unsigned char));

	memcpy(curBytes, &pLength, sizeof(int));
	index = sizeof(int);
	memcpy(curBytes + index, pBytes, pLength);
	index += pLength;

	memcpy(curBytes + index, &gLength, sizeof(int));
	index += sizeof(int);
	memcpy(curBytes + index , gBytes, gLength);
	index += gLength;

	memcpy(curBytes + index, &hLength, sizeof(int));
	index += sizeof(int);
	memcpy(curBytes + index, hBytes, hLength);


	sendBoth(writeSocket, (octet*) curBytes, curLength);

	free(curBytes);
	free(pBytes);
	free(gBytes);
	free(hBytes);

	return 1;
}


struct commit_batch_params *receive_commit_batch_params(int writeSocket, int readSocket)
{
	struct commit_batch_params *params = init_commit_batch_params();
	int curLength, pLength, gLength, hLength, index = sizeof(int);
	unsigned char *curBytes;
	
	mpz_t *tempMPZ = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_init(*tempMPZ);

	curBytes = receiveBoth(readSocket, curLength);


	memcpy(&pLength, curBytes, sizeof(int));
	convertBytesToMPZ(tempMPZ, curBytes + index, pLength);
	mpz_set(params -> group -> p, *tempMPZ);
	index += pLength;

	memcpy(&gLength, curBytes + index, sizeof(int));
	index += sizeof(int);
	convertBytesToMPZ(tempMPZ, curBytes + index, gLength);
	mpz_set(params -> group -> g, *tempMPZ);
	index += gLength;

	memcpy(&hLength, curBytes + index, sizeof(int));
	index += sizeof(int);
	convertBytesToMPZ(tempMPZ, curBytes + index, hLength);
	mpz_set(params -> h, *tempMPZ);

	free(curBytes);
	free(tempMPZ);

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