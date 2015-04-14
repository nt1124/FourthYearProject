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
	mpz_init(group -> q);

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
	mpz_t candiateG;

	mpz_init(candiateG);

	getSafePrimeGMP(group -> p, group -> q, state, securityParam);

	do
	{
		mpz_urandomm(candiateG, state, group -> p);
		mpz_powm_ui(group -> g, candiateG, 2, group -> p);
	} while( 0 == mpz_cmp_ui(group -> g, 1) || 0 == mpz_cmp_ui(group -> g, 0)   );


	return group;
}


// SecurityParam is the number of bits for p.
struct DDH_Group *getSchnorrGroup(int securityParam, gmp_randstate_t state)
{
	const char *pStr = "f65c35a65bc8cf3a24c459608f6d4cf2c6d0620912d2a70e44c98b0d5e5c410a3aec63412416f7da6904f41457a0c353d2394d565490fb2a7ee00a367d9ca152ab406d5f75a4059dddadb1a48c8193acf9fc454538948f314752f00431c97897b89b88c362d7177a12db635ce97a4db56e7d8a823029c5c946a3b824e4718daf";
	const char *qStr = "7b2e1ad32de4679d12622cb047b6a67963683104896953872264c586af2e20851d7631a0920b7bed34827a0a2bd061a9e91ca6ab2a487d953f70051b3ece50a955a036afbad202ceeed6d8d24640c9d67cfe22a29c4a4798a3a9780218e4bc4bdc4dc461b16b8bbd096db1ae74bd26dab73ec5411814e2e4a351dc127238c6d7";

	struct DDH_Group *group = initGroupStruct();
	mpz_t candiateG;

	mpz_init(candiateG);

	// getSafePrimeGMP(group -> p, group -> q, state, securityParam);
	gmp_sscanf (pStr, "%Zx", group -> p);
	gmp_sscanf (qStr, "%Zx", group -> q);

	do
	{
		mpz_urandomm(candiateG, state, group -> p);
		mpz_powm_ui(group -> g, candiateG, 2, group -> p);
	} while( 0 == mpz_cmp_ui(group -> g, 1) || 0 == mpz_cmp_ui(group -> g, 0)   );


	return group;
}


// SecurityParam is the number of bits for p.
struct DDH_Group *get_128_Bit_Group(gmp_randstate_t state)
{
	const char *pStr = "1cf2649bd19176df80f61bd40cd6c994f";
	const char *qStr = "e79324de8c8bb6fc07b0dea066b64ca7";

	struct DDH_Group *group = initGroupStruct();
	mpz_t candiateG;

	mpz_init(candiateG);

	// getSafePrimeGMP(group -> p, group -> q, state, securityParam);
	gmp_sscanf (pStr, "%Zx", group -> p);
	gmp_sscanf (qStr, "%Zx", group -> q);

	do
	{
		mpz_urandomm(candiateG, state, group -> p);
		// mpz_set_ui(candiateG, 4);
		mpz_powm_ui(group -> g, candiateG, 2, group -> p);
	} while( 0 == mpz_cmp_ui(group -> g, 1) || 0 == mpz_cmp_ui(group -> g, 0)   );


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
	mpz_t finalFactor, y_Inv, M_y;

	mpz_init(*M);
	mpz_init(M_y);
	mpz_init(M_unmodded);
	mpz_init(c1_sk);
	mpz_init(c1_sk_inv);
	mpz_init(finalFactor);
	mpz_init(y_Inv);


	if(0x00 == sigmaBit)
	{
		mpz_invert(y_Inv, y, group -> q);

		mpz_mul(finalFactor, y_Inv, *sk);
		mpz_mod(y_Inv, finalFactor, group -> q);

		mpz_powm(c1_sk, C -> u, y_Inv, group -> p);
		mpz_invert(c1_sk_inv, c1_sk, group -> p);

		mpz_mul(M_unmodded, c1_sk_inv, C -> v);
		mpz_mod(*M, M_unmodded, group -> p);
	}
	else
	{
		mpz_powm(c1_sk, C -> u, *sk, group -> p);
		mpz_invert(finalFactor, c1_sk, group -> p);

		mpz_powm(c1_sk_inv, finalFactor, y, group -> p);

		mpz_mul(M_unmodded, c1_sk_inv, C -> v);
		mpz_mod(*M, M_unmodded, group -> p);
	}

	return M;
}



// Functions below here deal with sending/serialising DDH groups.

int sendDDH_Group(int writeSocket, int readSocket, struct DDH_Group *group)
{
	unsigned char *curBytes, *pBytes, *gBytes, *qBytes;
	int curLength, pLength, gLength, qLength, index;


	pBytes = convertMPZToBytes( group -> p, &pLength);
	gBytes = convertMPZToBytes( group -> g, &gLength);
	qBytes = convertMPZToBytes( group -> q, &qLength);

	curLength = 3 * sizeof(int) + pLength + gLength + qLength;
	curBytes = (unsigned char*) calloc(curLength, sizeof(unsigned char));

	memcpy(curBytes, &pLength, sizeof(int));
	index = sizeof(int);
	memcpy(curBytes + index, pBytes, pLength);
	index += pLength;

	memcpy(curBytes + index, &gLength, sizeof(int)); 
	index += sizeof(int);
	memcpy(curBytes + index, gBytes, gLength);
	index += gLength;

	memcpy(curBytes + index, &qLength, sizeof(int)); 
	index += sizeof(int);
	memcpy(curBytes + index, qBytes, qLength);
	index += qLength;

	sendBoth(writeSocket, (octet*) curBytes, curLength);

	free(curBytes);
	free(pBytes);
	free(gBytes);
	free(qBytes);

	return 1;
}


struct DDH_Group *receiveDDH_Group(int writeSocket, int readSocket)
{
	struct DDH_Group *group = initGroupStruct();
	unsigned char *curBytes;
	int curLength, pLength, gLength, qLength, index;
	
	mpz_t *tempMPZ = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_init(*tempMPZ);

	curBytes = receiveBoth(readSocket, curLength);

	memcpy(&pLength, curBytes, sizeof(int));
	index = sizeof(int);
	convertBytesToMPZ(tempMPZ, curBytes + index, pLength);
	index += pLength;
	mpz_set(group -> p, *tempMPZ);

	memcpy(&gLength, curBytes + index, sizeof(int));
	index += sizeof(int);
	convertBytesToMPZ(tempMPZ, curBytes + index, gLength);
	index += gLength;
	mpz_set(group -> g, *tempMPZ);

	memcpy(&qLength, curBytes + index, sizeof(int));
	index += sizeof(int);
	convertBytesToMPZ(tempMPZ, curBytes + index, qLength);
	index += qLength;
	mpz_set(group -> q, *tempMPZ);

	free(curBytes);
	free(tempMPZ);

	return group;
}



unsigned char *serialiseDDH_Group(struct DDH_Group *group, int *bufferLength)
{
	unsigned char *curBytes, *pBytes, *gBytes, *qBytes;
	int curLength = 0, pLength = 0, gLength = 0, qLength = 0, index = 0;

	pBytes = convertMPZToBytes( group -> p, &pLength);
	gBytes = convertMPZToBytes( group -> g, &gLength);
	qBytes = convertMPZToBytes( group -> q, &qLength);

	curLength = 3 * sizeof(int) + pLength + gLength + qLength;
	curBytes = (unsigned char*) calloc(curLength, sizeof(unsigned char));

	memcpy(curBytes, &pLength, sizeof(int));
	index = sizeof(int);
	memcpy(curBytes + index, pBytes, pLength);
	index += pLength;

	memcpy(curBytes + index, &gLength, sizeof(int));
	index += sizeof(int);
	memcpy(curBytes + index, gBytes, gLength);
	index += gLength;

	memcpy(curBytes + index, &qLength, sizeof(int));
	index += sizeof(int);
	memcpy(curBytes + index, qBytes, qLength);
	index += qLength;

	free(pBytes);
	free(gBytes);
	free(qBytes);

	*bufferLength = curLength;

	return curBytes;
}


struct DDH_Group *deserialise_DDH_Group(unsigned char *curBytes, int *bufferOffset)
{
	struct DDH_Group *group = initGroupStruct();
	int pLength, gLength, qLength;
	
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


	memcpy(&qLength, curBytes + *bufferOffset, sizeof(int));
	*bufferOffset += sizeof(int);

	convertBytesToMPZ(tempMPZ, curBytes + *bufferOffset, qLength);
	mpz_set(group -> q, *tempMPZ);
	*bufferOffset += qLength;


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
