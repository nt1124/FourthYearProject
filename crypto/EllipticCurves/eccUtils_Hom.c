struct eccPoint_Hom *initECC_Point_Hom()
{
	struct eccPoint_Hom *output = (struct eccPoint_Hom *) calloc(1, sizeof(struct eccPoint_Hom));

	mpz_init(output -> x);
	mpz_init(output -> y);
	mpz_init(output -> z);

	output -> pointAtInf = 0;

	return output;
}


struct eccPoint_Hom *init_Identity_ECC_Point_Hom()
{
	struct eccPoint_Hom *output = (struct eccPoint_Hom *) calloc(1, sizeof(struct eccPoint_Hom));

	mpz_init_set_ui(output -> x, 0);
	mpz_init_set_ui(output -> y, 1);
	mpz_init_set_ui(output -> z, 0);

	output -> pointAtInf = 1;

	return output;
}


struct eccPoint_Hom *initAndSetECC_Point_Hom(mpz_t x, mpz_t y, mpz_t z, unsigned char identity)
{
	struct eccPoint_Hom *output = (struct eccPoint_Hom *) calloc(1, sizeof(struct eccPoint_Hom));

	mpz_init_set(output -> x, x);
	mpz_init_set(output -> y, y);
	mpz_init_set(output -> z, z);

	output -> pointAtInf = identity;


	return output;
}


struct eccPoint_Hom *copyECC_Point_Hom(struct eccPoint_Hom *toCopy)
{
	struct eccPoint_Hom *output = (struct eccPoint_Hom *) calloc(1, sizeof(struct eccPoint_Hom));

	mpz_init_set(output -> x, toCopy -> x);
	mpz_init_set(output -> y, toCopy -> y);
	mpz_init_set(output -> z, toCopy -> z);

	output -> pointAtInf = toCopy -> pointAtInf;

	return output;
}



void clearECC_Point_Hom(struct eccPoint_Hom *toClear)
{
	mpz_clear(toClear -> x);
	mpz_clear(toClear -> y);
	mpz_clear(toClear -> z);

	free(toClear);
}


void printPoint_Hom(struct eccPoint_Hom *P)
{
	if(0 == P -> pointAtInf)
	{
		gmp_printf("(%Zd, %Zd, %Zd)\n", P -> x, P -> y, P -> z);
	}
	else
	{
		printf("(@, @, @)\n");
	}
}


struct eccParams_Hom *initECC_Params_Hom()
{
	struct eccParams_Hom *output = (struct eccParams_Hom *) calloc(1, sizeof(struct eccParams_Hom));

	mpz_init(output -> p);
	mpz_init(output -> a);
	mpz_init(output -> b);
	output -> g = initECC_Point_Hom();
	mpz_init(output -> n);

	return output;
}


struct eccParams_Hom *initAndSetECC_Params_Hom(mpz_t p, mpz_t a, mpz_t b, struct eccPoint_Hom *g, mpz_t n)
{
	struct eccParams_Hom *output = (struct eccParams_Hom *) calloc(1, sizeof(struct eccParams_Hom));


	mpz_init_set(output -> p, p);
	mpz_init_set(output -> a, a);
	mpz_init_set(output -> b, b);
	mpz_init_set(output -> n, n);

	output -> g = g;


	return output;
}


struct ecc_Ciphertext_Hom *initECC_Ciphertext_Hom()
{
	struct ecc_Ciphertext_Hom *output = (struct ecc_Ciphertext_Hom *) calloc(1, sizeof(struct ecc_Ciphertext_Hom));

	output -> sessionKey = initECC_Point_Hom();
	output -> msgPart = initECC_Point_Hom();


	return output;
}


// Initialise the dec side of the params
struct ECC_PK_Hom *initPK_For_Use_Hom()
{
	struct ECC_PK_Hom *pk = (struct ECC_PK_Hom*) calloc(1, sizeof(struct ECC_PK_Hom));


	pk -> g = initECC_Point_Hom();
	pk -> g_x = initECC_Point_Hom();
	pk -> h = initECC_Point_Hom();
	pk -> h_x = initECC_Point_Hom();


	return pk;
}


int sizeOfSerial_ECCPoint_Hom(struct eccPoint_Hom *P)
{
	int totalLength = sizeof(int) * 3 + 1;

	totalLength += sizeof(mp_limb_t) * (mpz_size(P -> x) + mpz_size(P -> y) + mpz_size(P -> z));

	
	return totalLength;
}



void serialise_ECC_Point_Hom(struct eccPoint_Hom *P, unsigned char *outputBuffer, int *outputOffset)
{

	int offset = *outputOffset;

	serialiseMPZ(P -> x, outputBuffer, &offset);
	serialiseMPZ(P -> y, outputBuffer, &offset);
	serialiseMPZ(P -> z, outputBuffer, &offset);
	memcpy(outputBuffer + offset, &(P -> pointAtInf), sizeof(unsigned char));


	*outputOffset = offset + 1;
}


struct eccPoint_Hom *deserialise_ECC_Point_Hom(unsigned char *inputBuffer, int *inputOffset)
{
	struct eccPoint_Hom *output = initECC_Point_Hom();
	int offset = *inputOffset;
	mpz_t *tempMPZ;


	tempMPZ = deserialiseMPZ(inputBuffer, &offset);
	mpz_set(output -> x, *tempMPZ);
	free(tempMPZ);

	tempMPZ = deserialiseMPZ(inputBuffer, &offset);
	mpz_set(output -> y, *tempMPZ);
	free(tempMPZ);

	tempMPZ = deserialiseMPZ(inputBuffer, &offset);
	mpz_set(output -> z, *tempMPZ);
	free(tempMPZ);

	memcpy(&(output -> pointAtInf), inputBuffer + offset, sizeof(unsigned char));


	*inputOffset = offset + 1;

	return output;
}



unsigned char *serialiseECC_Params_Hom(struct eccParams_Hom *params, int *outputLen)
{
	unsigned char *outputBuffer;
	int totalLength = 4 * sizeof(int), offset = 0;

	totalLength += sizeof(mp_limb_t) * mpz_size(params -> p);
	totalLength += sizeof(mp_limb_t) * mpz_size(params -> a);
	totalLength += sizeof(mp_limb_t) * mpz_size(params -> b);
	totalLength += sizeof(mp_limb_t) * mpz_size(params -> n);
	totalLength += sizeOfSerial_ECCPoint_Hom(params -> g);


	outputBuffer = (unsigned char *) calloc(totalLength, sizeof(unsigned char));
	serialiseMPZ(params -> p, outputBuffer, &offset);
	serialiseMPZ(params -> a, outputBuffer, &offset);
	serialiseMPZ(params -> b, outputBuffer, &offset);
	serialiseMPZ(params -> n, outputBuffer, &offset);
	serialise_ECC_Point_Hom(params -> g, outputBuffer, &offset);


	*outputLen = offset;

	return outputBuffer;
}


struct eccParams_Hom *deserialiseECC_Params_Hom(unsigned char *inputBuffer, int *inputOffset)
{
	struct eccParams_Hom *output = initECC_Params_Hom();
	int offset = *inputOffset;
	mpz_t *tempMPZ;


	tempMPZ = deserialiseMPZ(inputBuffer, &offset);
	mpz_set(output -> p, *tempMPZ);
	free(tempMPZ);

	tempMPZ = deserialiseMPZ(inputBuffer, &offset);
	mpz_set(output -> a, *tempMPZ);
	free(tempMPZ);

	tempMPZ = deserialiseMPZ(inputBuffer, &offset);
	mpz_set(output -> b, *tempMPZ);
	free(tempMPZ);

	tempMPZ = deserialiseMPZ(inputBuffer, &offset);
	mpz_set(output -> n, *tempMPZ);
	free(tempMPZ);

	output -> g = deserialise_ECC_Point_Hom(inputBuffer, &offset);


	*inputOffset = offset;

	return output;
}



void serialise_U_V_pair_ECC_Hom(struct u_v_Pair_ECC_Hom *c, unsigned char *outputBuffer, int *bufferOffset)
{
	int outputOffset = *bufferOffset;


	serialise_ECC_Point_Hom(c -> u, outputBuffer, &outputOffset);
	serialise_ECC_Point_Hom(c -> v, outputBuffer, &outputOffset);


	*bufferOffset = outputOffset;
}


struct u_v_Pair_ECC_Hom *deserialise_U_V_pair_ECC_Hom(unsigned char *inputBuffer, int *bufferOffset)
{
	struct u_v_Pair_ECC_Hom *c = (struct u_v_Pair_ECC_Hom*) calloc(1, sizeof(u_v_Pair_ECC_Hom));
	int tempOffset = *bufferOffset;


	c -> u = deserialise_ECC_Point_Hom(inputBuffer, &tempOffset);	
	c -> v = deserialise_ECC_Point_Hom(inputBuffer, &tempOffset);


	*bufferOffset = tempOffset;

	return c;
}


unsigned char *serialise_U_V_Pair_ECC_Hom_Array(struct u_v_Pair_ECC_Hom **inputs, int num_u_v_Pairs, int *outputLength)
{
	unsigned char *outputBuffer;
	int totalLength = 0;
	int i, bufferOffset = 0;


	for(i = 0; i < num_u_v_Pairs; i ++)
	{
		totalLength += sizeOfSerial_ECCPoint_Hom(inputs[i] -> u);
		totalLength += sizeOfSerial_ECCPoint_Hom(inputs[i] -> v);
	}

	outputBuffer = (unsigned char *) calloc(totalLength, sizeof(unsigned char));

	for(i = 0; i < num_u_v_Pairs; i ++)
	{
		serialise_U_V_pair_ECC_Hom(inputs[i], outputBuffer, &bufferOffset);
	}

	*outputLength = totalLength;

	return outputBuffer;
}


struct u_v_Pair_ECC_Hom **deserialise_U_V_Pair_ECC_Hom_Array(unsigned char *inputBuffer, int num_u_v_Pairs)
{
	struct u_v_Pair_ECC_Hom **outputKeys = (struct u_v_Pair_ECC_Hom **) calloc(num_u_v_Pairs, sizeof(struct u_v_Pair_ECC_Hom*));
	int i, bufferOffset = 0;


	for(i = 0; i < num_u_v_Pairs; i ++)
	{
		outputKeys[i] = deserialise_U_V_pair_ECC_Hom(inputBuffer, &bufferOffset);
	}

	return outputKeys;
}