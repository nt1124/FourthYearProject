// Classic Sigma proof of knowledge of a Discrete Logarithm
int ZKPoK_DL_Prover(int writeSocket, int readSocket,
					struct eccPoint *g_0, struct eccPoint *g_1, mpz_t x,
					struct eccParams *params, gmp_randstate_t state)
{
	struct eccPoint *H;
	unsigned char *commBuffer;
	mpz_t r, s, sUnmodded, *c;
	int bufferOffset = 0, commBufferLen = 0;


	mpz_init(s);
	mpz_init(r);
	mpz_urandomm(r, state, params -> n);
	mpz_init_set(sUnmodded, r);

	H = windowedScalarPoint(r, g_0, params);
	commBuffer = (unsigned char *) calloc(sizeOfSerial_ECCPoint(H), sizeof(unsigned char));
	serialise_ECC_Point(H, commBuffer, &bufferOffset);
	sendBoth(writeSocket, commBuffer, bufferOffset);
	free(commBuffer);


	commBuffer = receiveBoth(readSocket, commBufferLen);
	bufferOffset = 0;
	c = deserialiseMPZ(commBuffer, &bufferOffset);

	bufferOffset = 0;
	mpz_addmul(sUnmodded, *c, x);
	mpz_mod(s, sUnmodded, params -> n);

	commBufferLen = sizeof(int) + (sizeof(mp_limb_t) * mpz_size(s));
	commBuffer = (unsigned char *) calloc(commBufferLen, sizeof(unsigned char));
	serialiseMPZ(s, commBuffer, &bufferOffset);
	sendBoth(writeSocket, commBuffer, bufferOffset);
	free(commBuffer);


	clearECC_Point(H);

	return 1;
}



int ZKPoK_DL_Verifier(int writeSocket, int readSocket, struct eccParams *params,
					struct eccPoint *g_0, struct eccPoint *g_1, gmp_randstate_t state)
{
	struct eccPoint *H, *h_g_1PowC, *g_0PowS, finalH;
	mpz_t C, *s;
	unsigned char *commBuffer;
	int commBufferLen, bufferOffset = 0, verified = 0;


	mpz_init(C);

	mpz_urandomm(C, state, params -> n);


	commBuffer = receiveBoth(readSocket, commBufferLen);
	H = deserialise_ECC_Point(commBuffer, &bufferOffset);
	free(commBuffer);

	bufferOffset = 0;
	commBufferLen = sizeof(int) + (sizeof(mp_limb_t) * mpz_size(C));
	commBuffer = (unsigned char *) calloc(commBufferLen, sizeof(unsigned char));
	serialiseMPZ(C, commBuffer, &bufferOffset);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);


	bufferOffset = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	s = deserialiseMPZ(commBuffer, &bufferOffset);
	free(commBuffer);

	g_0PowS = windowedScalarPoint(*s, g_0, params);
	h_g_1PowC = windowedScalarPoint(C, g_1, params);
	groupOp_PlusEqual(h_g_1PowC, H, params);


	verified = eccPointsEqual(g_0PowS, h_g_1PowC);

	clearECC_Point(g_0PowS);
	clearECC_Point(h_g_1PowC);
	clearECC_Point(H);


	return verified;
}