struct u_v_Pair *init_U_V()
{
	struct u_v_Pair *u_v = (struct u_v_Pair*) calloc(1, sizeof(struct u_v_Pair));

	mpz_init(u_v -> u);
	mpz_init(u_v -> v);

	return u_v;
}


struct DDH_Group *initGroupStruct()
{
	struct DDH_Group *group = (struct DDH_Group*) calloc(1, sizeof(struct DDH_Group));

	mpz_init(group -> p);
	mpz_init(group -> g);

	return group;
}


struct DDH_PK *initPublicKey()
{
	struct DDH_PK *pk = (struct DDH_PK*) calloc(1, sizeof(struct DDH_PK));

	mpz_init(pk -> g);
	mpz_init(pk -> h);
	mpz_init(pk -> g_x);
	mpz_init(pk -> h_x);

	return pk;
}



// Used to generate some randomness for use in encryption.
struct u_v_Pair *randomiseDDH(struct DDH_PK *pk, struct DDH_Group *group, gmp_randstate_t state)
{
	struct u_v_Pair *toReturn = (struct u_v_Pair*) calloc(1, sizeof(struct u_v_Pair));
	mpz_t s, t;
	mpz_t tempPowS, tempPowT;
	mpz_t temp;

	mpz_init(s);
	mpz_init(t);
	mpz_init(temp);
	mpz_init(tempPowS);
	mpz_init(tempPowT);
	mpz_init(toReturn -> u);
	mpz_init(toReturn -> v);

	mpz_urandomm(s, state, group -> p);
	mpz_urandomm(t, state, group -> p);

	mpz_powm(tempPowS, pk -> g, s, group -> p);
	mpz_powm(tempPowT, pk -> h, t, group -> p);
	mpz_mul(temp, tempPowS, tempPowT);
	mpz_mod(toReturn -> u, temp, group -> p);

	mpz_powm(tempPowS, pk -> g_x, s, group -> p);
	mpz_powm(tempPowT, pk -> h_x, t, group -> p);
	mpz_mul(temp, tempPowS, tempPowT);
	mpz_mod(toReturn -> v, temp, group -> p);

	return toReturn;
}


// SecurityParam is the number of bits for p.
struct DDH_Group *generateGroup(int securityParam, gmp_randstate_t state)
{
	struct DDH_Group *group = initGroupStruct();

	getPrimeGMP(group -> p, state, securityParam);
	// getSafePrimeGMP(group -> p, state, securityParam);
	
	do
	{
		mpz_urandomm(group -> g, state, group -> p);
	} while( 0 > mpz_cmp_ui(group -> g, 1) );

	return group;
}


// Generate the keys given a group.
struct DDH_KeyPair *generateKeys(struct DDH_Group *group, gmp_randstate_t state)
{
	struct DDH_KeyPair *DDH_keyPair = (struct DDH_KeyPair*) calloc(1, sizeof(struct DDH_KeyPair));
	DDH_keyPair -> pk = initPublicKey();
	DDH_keyPair -> sk = (DDH_SK*) calloc(1, sizeof(mpz_t));
	mpz_init( *(DDH_keyPair -> sk) );

	mpz_set(DDH_keyPair -> pk -> g, group -> g);
	do
	{
		mpz_urandomm(DDH_keyPair -> pk -> h, state, group -> p);
	} while( 0 > mpz_cmp_ui(DDH_keyPair -> pk -> h, 1) );

	do
	{
		mpz_urandomm( *(DDH_keyPair -> sk), state, group -> p);
	} while( 0 > mpz_cmp_ui( *(DDH_keyPair -> sk), 1) );

	mpz_powm(DDH_keyPair -> pk -> g_x, DDH_keyPair -> pk -> g, *(DDH_keyPair -> sk), group -> p);
	mpz_powm(DDH_keyPair -> pk -> h_x, DDH_keyPair -> pk -> h, *(DDH_keyPair -> sk), group -> p);

	return DDH_keyPair;
}


struct u_v_Pair *encDDH(struct DDH_PK *pk, struct DDH_Group *group, mpz_t m, gmp_randstate_t state)
{
	struct u_v_Pair *C = randomiseDDH(pk, group, state);

	// Temp variable to hold value of v*m before modulo applied. 
	mpz_t tempVM;
	mpz_init(tempVM);

	mpz_mul(tempVM, C -> v, m);
	mpz_mod(C -> v, tempVM, group -> p);

	return C;
}


// (c_0, c_1) = (C -> u, C -> v)
mpz_t *decDDH(DDH_SK *sk, struct DDH_Group *group, struct u_v_Pair *C)
{
	mpz_t *M = (mpz_t *) calloc(1, sizeof(mpz_t));
	mpz_t c1_sk, c1_sk_inv;
	mpz_t M_unmodded;

	mpz_init(*M);
	mpz_init(M_unmodded);
	mpz_init(c1_sk);
	mpz_init(c1_sk_inv);

	mpz_powm(c1_sk, C -> u, *sk, group -> p);

	mpz_invert(c1_sk_inv, c1_sk, group -> p);

	// M = c_1 * (c_0 ^ sk) ^ -1
	mpz_mul(M_unmodded, c1_sk_inv, C -> v);
	mpz_mod(*M, M_unmodded, group -> p);

	return M;
}


// (c_0, c_1) = (C -> u, C -> v)
mpz_t *decDDH_Alt(DDH_SK *sk, mpz_t y, struct DDH_Group *group, struct u_v_Pair *C, unsigned char sigmaBit)
{
	mpz_t *M = (mpz_t *) calloc(1, sizeof(mpz_t));
	mpz_t c1_sk, c1_sk_inv;
	mpz_t M_unmodded;
	mpz_t finalFactor, temp;

	mpz_init(*M);
	mpz_init(M_unmodded);
	mpz_init(c1_sk);
	mpz_init(c1_sk_inv);
	mpz_init(finalFactor);
	mpz_init(temp);


	if(0x00 == sigmaBit)
	{
		mpz_invert(temp, y, group -> p);
	}
	else
	{
		mpz_set(temp, y);
	}

	mpz_mul(finalFactor, C -> u, temp);
	mpz_mod(temp, finalFactor, group -> p);
	mpz_powm(c1_sk, temp, *sk, group -> p);

	mpz_invert(c1_sk_inv, c1_sk, group -> p);

	mpz_mul(M_unmodded, c1_sk_inv, C -> v);
	mpz_mod(*M, M_unmodded, group -> p);

	return M;
}



// Functions below here deal with sending/serialising DDH groups.

int sendDDH_Group(int writeSocket, int readSocket, struct DDH_Group *group)
{
	unsigned char *curBytes, *pBytes, *gBytes;
	int curLength, pLength, gLength;


	pBytes = convertMPZToBytes( group -> p, &pLength);
	gBytes = convertMPZToBytes( group -> g, &gLength);

	curLength = pLength + gLength;
	curBytes = (unsigned char*) calloc(2 * sizeof(int) + curLength, sizeof(unsigned char));

	memcpy(curBytes, &pLength, sizeof(int));
	memcpy(curBytes + sizeof(int), pBytes, pLength);

	memcpy(curBytes + sizeof(int) + pLength, &gLength, sizeof(int));
	memcpy(curBytes + 2*sizeof(int) + pLength, gBytes, gLength);

	sendBoth(writeSocket, (octet*) curBytes, curLength);

	free(curBytes);
	free(pBytes);
	free(gBytes);

	return 1;
}


struct DDH_Group *receiveDDH_Group(int writeSocket, int readSocket)
{
	struct DDH_Group *group = initGroupStruct();
	unsigned char *curBytes;
	int curLength, pLength, gLength;
	
	mpz_t *tempMPZ = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_init(*tempMPZ);

	curBytes = receiveBoth(readSocket, curLength);

	memcpy(&pLength, curBytes, sizeof(int));
	convertBytesToMPZ(tempMPZ, curBytes + sizeof(int), pLength);
	mpz_set(group -> p, *tempMPZ);

	memcpy(&gLength, curBytes + sizeof(int) + pLength, sizeof(int));
	convertBytesToMPZ(tempMPZ, curBytes + 2*sizeof(int) + pLength, gLength);
	mpz_set(group -> g, *tempMPZ);

	free(curBytes);
	free(tempMPZ);

	return group;
}



unsigned char *serialiseDDH_Group(struct DDH_Group *group, int *bufferLength)
{
	unsigned char *curBytes, *pBytes, *gBytes;
	int curLength = 0, pLength = 0, gLength = 0;

	pBytes = convertMPZToBytes( group -> p, &pLength);
	gBytes = convertMPZToBytes( group -> g, &gLength);

	curLength = 2 * sizeof(int) + pLength + gLength;
	curBytes = (unsigned char*) calloc(curLength, sizeof(unsigned char));

	memcpy(curBytes, &pLength, sizeof(int));
	memcpy(curBytes + sizeof(int), pBytes, pLength);

	memcpy(curBytes + sizeof(int) + pLength, &gLength, sizeof(int));
	memcpy(curBytes + 2*sizeof(int) + pLength, gBytes, gLength);

	free(pBytes);
	free(gBytes);

	*bufferLength = 2*sizeof(int) + pLength + gLength;

	return curBytes;
}


struct DDH_Group *deserialise_DDH_Group(unsigned char *curBytes, int *bufferOffset)
{
	struct DDH_Group *group = initGroupStruct();
	int pLength, gLength;
	
	mpz_t *tempMPZ = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_init(*tempMPZ);


	memcpy(&pLength, curBytes + *bufferOffset, sizeof(int));
	*bufferOffset += sizeof(int);

	convertBytesToMPZ(tempMPZ, curBytes + *bufferOffset, pLength);
	mpz_set(group -> p, *tempMPZ);
	*bufferOffset += pLength;


	memcpy(&gLength, curBytes + *bufferOffset, sizeof(int));
	*bufferOffset += sizeof(int);

	convertBytesToMPZ(tempMPZ, curBytes + *bufferOffset, gLength);
	mpz_set(group -> g, *tempMPZ);
	*bufferOffset += gLength;


	free(tempMPZ);

	return group;
}



int test_DDH_Local()
{
	mpz_t *input0 = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_t *decSigma = (mpz_t*) calloc(1, sizeof(mpz_t));

	struct u_v_Pair *y0;
	struct DDH_Group *group = initGroupStruct();
	struct DDH_KeyPair *keyPair;

	gmp_randstate_t *state = seedRandGen();
	group = generateGroup(1024, *state);

	mpz_urandomm(*input0, *state, group -> p);
	keyPair = generateKeys(group, *state);

	y0 = encDDH(keyPair -> pk, group, *input0, *state);
	decSigma = decDDH(keyPair -> sk, group, y0);

	printf("-- %d\n", mpz_cmp(*decSigma, *input0));

	return 0;
}
