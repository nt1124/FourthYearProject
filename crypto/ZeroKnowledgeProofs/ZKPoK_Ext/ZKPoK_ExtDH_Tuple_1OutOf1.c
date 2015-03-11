#ifndef ZKPoK_EXT_DH_TUPLE_ALL
#define ZKPoK_EXT_DH_TUPLE_ALL


void prove_SingleDH_Tuple(int writeSocket, int readSocket,
						mpz_t witness,
						struct eccPoint *g_0, struct eccPoint *g_1,
						struct eccPoint *u, struct eccPoint *v,
						struct eccParams *params, gmp_randstate_t *state)
{
	unsigned char *commBuffer;
	struct eccPoint *a, *b;
	int i, commBufferLen = 0, bufferOffset = 0, length;
	mpz_t r, *e, unmodded, eW, finalReply;


	mpz_init(r);
	mpz_init(unmodded);
	mpz_init(eW);
	mpz_init(finalReply);
	mpz_urandomm(r, *state, params -> n);


	a = windowedScalarPoint(r, g_0, params);
	b = windowedScalarPoint(r, g_1, params);

	commBufferLen = sizeOfSerial_ECCPoint(a) + sizeOfSerial_ECCPoint(b);
	commBuffer = (unsigned char *) calloc(commBufferLen, sizeof(unsigned char));
	serialise_ECC_Point(a, commBuffer, &bufferOffset);
	serialise_ECC_Point(b, commBuffer, &bufferOffset);
	sendBoth(writeSocket, commBuffer, bufferOffset);
	free(commBuffer);


	bufferOffset = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	e = deserialiseMPZ(commBuffer, &bufferOffset);
	free(commBuffer);


	mpz_mul(eW, *e, witness);
	mpz_add(unmodded, eW, r);
	mpz_add(finalReply, unmodded, params -> n);

	bufferOffset = 0;
	commBufferLen = sizeof(int) + (sizeof(mp_limb_t) * mpz_size(finalReply));
	commBuffer = (unsigned char *) calloc(commBufferLen, sizeof(unsigned char));
	serialiseMPZ(finalReply, commBuffer, &bufferOffset);
	sendBoth(writeSocket, commBuffer, bufferOffset);
	free(commBuffer);


	mpz_clear(r);
	mpz_clear(unmodded);
	mpz_clear(eW);
	mpz_clear(finalReply);
	mpz_clear(*e);
	free(e);
}



int verify_SingleDH_Tuple(int writeSocket, int readSocket,
						struct eccPoint *g_0, struct eccPoint *g_1,
						struct eccPoint *u, struct eccPoint *v,
						struct eccParams *params, gmp_randstate_t *state)
{
	unsigned char *commBuffer;
	int i, commBufferLen = 0, bufferOffset = 0, verified = 0;
	struct eccPoint *a, *uCheck, *b, *vCheck, *x, *y;
	mpz_t e, *z;



	bufferOffset = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	a = deserialise_ECC_Point(commBuffer, &bufferOffset);
	b = deserialise_ECC_Point(commBuffer, &bufferOffset);
	free(commBuffer);

	mpz_init(e);
	mpz_urandomm(e, *state, params -> n);

	bufferOffset = 0;
	commBufferLen = sizeof(int) + (sizeof(mp_limb_t) * mpz_size(e));
	commBuffer = (unsigned char *) calloc(commBufferLen, sizeof(unsigned char));
	serialiseMPZ(e, commBuffer, &bufferOffset);
	sendBoth(writeSocket, commBuffer, bufferOffset);
	free(commBuffer);


	bufferOffset = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	z = deserialiseMPZ(commBuffer, &bufferOffset);
	free(commBuffer);


	uCheck = windowedScalarPoint(*z, g_0, params);
	vCheck = windowedScalarPoint(*z, g_1, params);

	x = windowedScalarPoint(e, u, params);
	y = windowedScalarPoint(e, v, params);

	groupOp_PlusEqual(a, x, params);
	groupOp_PlusEqual(b, y, params);


	verified = eccPointsEqual(a, uCheck) | eccPointsEqual(b, vCheck);

	return verified;
}


int ZKPoK_Ext_DH_TupleProverAll(int writeSocket, int readSocket, int numTuples, mpz_t *roeList, mpz_t *alphaList,
								struct eccPoint *g_0, struct eccPoint *g_1, struct eccPoint **inputList,
								struct eccParams *params, gmp_randstate_t *state)
{
	mpz_t witness;
	int i, j = 0;

	mpz_init(witness);

	for(i = 0; i < numTuples; i ++, j +=2)
	{
		mpz_mul(witness, roeList[i], alphaList[i]);
		prove_SingleDH_Tuple(writeSocket, readSocket, witness, g_0, g_1,
						inputList[j], inputList[j+1], params, state);
	}
}




int ZKPoK_Ext_DH_TupleVerifierAll(int writeSocket, int readSocket, int numTuples,
								struct eccPoint *g_0, struct eccPoint *g_1,
								struct eccPoint **inputList,
								struct eccParams *params, gmp_randstate_t *state)
{
	int i, j = 0, verified = 0;

	for(i = 0; i < numTuples; i ++, j +=2)
	{
		verified |= verify_SingleDH_Tuple(writeSocket, readSocket, g_0, g_1, inputList[j], inputList[j+1], params, state);
	}

	printf(">>>>> %d\n", verified);

	return verified;
}



#endif