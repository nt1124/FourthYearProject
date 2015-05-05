// Initialise an Elliptic Curve Point struct
struct eccPoint *initECC_Point()
{
	struct eccPoint *output = (struct eccPoint *) calloc(1, sizeof(struct eccPoint));

	mpz_init(output -> x);
	mpz_init(output -> y);

	output -> pointAtInf = 0;

	return output;
}


// Initialise the identity ECC point 
struct eccPoint *init_Identity_ECC_Point()
{
	struct eccPoint *output = (struct eccPoint *) calloc(1, sizeof(struct eccPoint));

	mpz_init(output -> x);
	mpz_init(output -> y);

	output -> pointAtInf = 1;

	return output;
}


// Take in an X and a Y and initialise an ECC point at (X, Y)
struct eccPoint *initAndSetECC_Point(mpz_t x, mpz_t y, unsigned char identity)
{
	struct eccPoint *output = (struct eccPoint *) calloc(1, sizeof(struct eccPoint));

	mpz_init_set(output -> x, x);
	mpz_init_set(output -> y, y);

	output -> pointAtInf = identity;


	return output;
}


// Copy and ECC Point
struct eccPoint *copyECC_Point(struct eccPoint *toCopy)
{
	struct eccPoint *output = (struct eccPoint *) calloc(1, sizeof(struct eccPoint));

	mpz_init_set(output -> x, toCopy -> x);
	mpz_init_set(output -> y, toCopy -> y);

	output -> pointAtInf = toCopy -> pointAtInf;

	return output;
}


// Clear an ECC Point memory.
void clearECC_Point(struct eccPoint *toClear)
{
	mpz_clear(toClear -> x);
	mpz_clear(toClear -> y);

	free(toClear);
}


// Ronseal
void printPoint(struct eccPoint *P)
{
	if(0 == P -> pointAtInf)
	{
		gmp_printf("(%Zd, %Zd)\n", P -> x, P -> y);
	}
	else
	{
		printf("(@, @)\n");
	}
}


// Init ECC Params generally
struct eccParams *initECC_Params()
{
	struct eccParams *output = (struct eccParams *) calloc(1, sizeof(struct eccParams));

	mpz_init(output -> p);
	mpz_init(output -> a);
	mpz_init(output -> b);
	output -> g = initECC_Point();
	mpz_init(output -> n);

	return output;
}


// Init ECC Params with given inputs.
struct eccParams *initAndSetECC_Params(mpz_t p, mpz_t a, mpz_t b, struct eccPoint *g, mpz_t n)
{
	struct eccParams *output = (struct eccParams *) calloc(1, sizeof(struct eccParams));


	mpz_init_set(output -> p, p);
	mpz_init_set(output -> a, a);
	mpz_init_set(output -> b, b);
	mpz_init_set(output -> n, n);

	output -> g = g;


	return output;
}


// Init an ECC Ciphertext
struct ecc_Ciphertext *initECC_Ciphertext()
{
	struct ecc_Ciphertext *output = (struct ecc_Ciphertext *) calloc(1, sizeof(struct ecc_Ciphertext));

	output -> sessionKey = initECC_Point();
	output -> msgPart = initECC_Point();


	return output;
}


// Initialise the dec side of the params
struct ECC_PK *initPK_For_Use()
{
	struct ECC_PK *pk = (struct ECC_PK*) calloc(1, sizeof(struct ECC_PK));


	pk -> g = initECC_Point();
	pk -> g_x = initECC_Point();
	pk -> h = initECC_Point();
	pk -> h_x = initECC_Point();


	return pk;
}


// How big (in uchars) is this input point serialised 
int sizeOfSerial_ECCPoint(struct eccPoint *P)
{
	int totalLength = sizeof(int) * 2 + 1;

	totalLength += sizeof(mp_limb_t) * (mpz_size(P -> x) + mpz_size(P -> y));

	
	return totalLength;
}


// Serialise the input ECC point placing the output into outputBuffer starting at outputOffset.
void serialise_ECC_Point(struct eccPoint *P, unsigned char *outputBuffer, int *outputOffset)
{

	int offset = *outputOffset;

	serialiseMPZ(P -> x, outputBuffer, &offset);
	serialiseMPZ(P -> y, outputBuffer, &offset);
	memcpy(outputBuffer + offset, &(P -> pointAtInf), sizeof(unsigned char));


	*outputOffset = offset + 1;
}


// Deserialise the input ECC point placing the output into outputBuffer starting at outputOffset. 
struct eccPoint *deserialise_ECC_Point(unsigned char *inputBuffer, int *inputOffset)
{
	struct eccPoint *output = initECC_Point();
	int offset = *inputOffset;
	mpz_t *tempMPZ;


	tempMPZ = deserialiseMPZ(inputBuffer, &offset);
	mpz_set(output -> x, *tempMPZ);
	mpz_clear(*tempMPZ);
	free(tempMPZ);

	tempMPZ = deserialiseMPZ(inputBuffer, &offset);
	mpz_set(output -> y, *tempMPZ);
	mpz_clear(*tempMPZ);
	free(tempMPZ);

	memcpy(&(output -> pointAtInf), inputBuffer + offset, sizeof(unsigned char));


	*inputOffset = offset + 1;

	return output;
}


unsigned char *serialise_ECC_Point_Array(struct eccPoint **inputArray, int arrayLen, int *outputLen)
{
	unsigned char *outputBuffer;
	int bufferOffset = sizeof(int), totalLength = sizeof(int), i;


	for(i = 0; i < arrayLen; i ++)
	{
		totalLength += sizeOfSerial_ECCPoint(inputArray[i]);
	}

	outputBuffer = (unsigned char*) calloc(totalLength, sizeof(unsigned char));

	memcpy(outputBuffer, &arrayLen, sizeof(int));
	for(i = 0; i < arrayLen; i ++)
	{
		serialise_ECC_Point(inputArray[i], outputBuffer, &bufferOffset);
	}


	*outputLen = bufferOffset;

	return outputBuffer;
}


struct eccPoint **deserialise_ECC_Point_Array(unsigned char *inputBuffer, int *arrayLen, int *inputOffset)
{
	struct eccPoint **output;
	int bufferOffset = *inputOffset, i;


	memcpy(arrayLen, inputBuffer + bufferOffset, sizeof(int));
	bufferOffset += sizeof(int);

	output = (struct eccPoint **) calloc(*arrayLen, sizeof(struct eccPoint *));

	for(i = 0; i < *arrayLen; i ++)
	{
		output[i] = deserialise_ECC_Point(inputBuffer, &bufferOffset);
	}

	*inputOffset = bufferOffset;

	return output;
}



unsigned char *serialiseECC_Params(struct eccParams *params, int *outputLen)
{
	unsigned char *outputBuffer;
	int totalLength = 4 * sizeof(int), offset = 0;

	totalLength += sizeof(mp_limb_t) * mpz_size(params -> p);
	totalLength += sizeof(mp_limb_t) * mpz_size(params -> a);
	totalLength += sizeof(mp_limb_t) * mpz_size(params -> b);
	totalLength += sizeof(mp_limb_t) * mpz_size(params -> n);
	totalLength += sizeOfSerial_ECCPoint(params -> g);


	outputBuffer = (unsigned char *) calloc(totalLength, sizeof(unsigned char));
	serialiseMPZ(params -> p, outputBuffer, &offset);
	serialiseMPZ(params -> a, outputBuffer, &offset);
	serialiseMPZ(params -> b, outputBuffer, &offset);
	serialiseMPZ(params -> n, outputBuffer, &offset);
	serialise_ECC_Point(params -> g, outputBuffer, &offset);


	*outputLen = offset;

	return outputBuffer;
}


struct eccParams *deserialiseECC_Params(unsigned char *inputBuffer, int *inputOffset)
{
	struct eccParams *output = initECC_Params();
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

	output -> g = deserialise_ECC_Point(inputBuffer, &offset);


	*inputOffset = offset;

	return output;
}



void serialise_U_V_pair_ECC(struct u_v_Pair_ECC *c, unsigned char *outputBuffer, int *bufferOffset)
{
	int outputOffset = *bufferOffset;


	serialise_ECC_Point(c -> u, outputBuffer, &outputOffset);
	serialise_ECC_Point(c -> v, outputBuffer, &outputOffset);


	*bufferOffset = outputOffset;
}


struct u_v_Pair_ECC *deserialise_U_V_pair_ECC(unsigned char *inputBuffer, int *bufferOffset)
{
	struct u_v_Pair_ECC *c = (struct u_v_Pair_ECC*) calloc(1, sizeof(u_v_Pair_ECC));
	int tempOffset = *bufferOffset;


	c -> u = deserialise_ECC_Point(inputBuffer, &tempOffset);	
	c -> v = deserialise_ECC_Point(inputBuffer, &tempOffset);


	*bufferOffset = tempOffset;

	return c;
}


unsigned char *serialise_U_V_Pair_ECC_Array(struct u_v_Pair_ECC **inputs, int num_u_v_Pairs, int *outputLength)
{
	unsigned char *outputBuffer;
	int totalLength = 2 * sizeof(int) * num_u_v_Pairs;
	int i, bufferOffset = 0;


	for(i = 0; i < num_u_v_Pairs; i ++)
	{
		totalLength += sizeOfSerial_ECCPoint(inputs[i] -> u);
		totalLength += sizeOfSerial_ECCPoint(inputs[i] -> v);
	}

	outputBuffer = (unsigned char *) calloc(totalLength, sizeof(unsigned char));

	for(i = 0; i < num_u_v_Pairs; i ++)
	{
		serialise_U_V_pair_ECC(inputs[i], outputBuffer, &bufferOffset);
	}

	*outputLength = totalLength;

	return outputBuffer;
}


struct u_v_Pair_ECC **deserialise_U_V_Pair_ECC_Array(unsigned char *inputBuffer, int num_u_v_Pairs)
{
	struct u_v_Pair_ECC **outputKeys = (struct u_v_Pair_ECC **) calloc(num_u_v_Pairs, sizeof(struct u_v_Pair_ECC*));
	int i, bufferOffset = 0;


	for(i = 0; i < num_u_v_Pairs; i ++)
	{
		outputKeys[i] = deserialise_U_V_pair_ECC(inputBuffer, &bufferOffset);
	}

	return outputKeys;
}


int eccPointsEqual(struct eccPoint *P, struct eccPoint *Q)
{
	int output = 1;

	if( 0 == mpz_cmp(P -> x, Q -> x) &&
		0 == mpz_cmp(P -> y, Q -> y) )
	{
		output = 0;
	}


	return output;
}


int checkValidECC_Point(struct eccPoint *P, struct eccParams *params)
{
	int output = 0;
	mpz_t temp1, temp2, temp3, candidateY_2, pY_2;


	mpz_init(temp1);
	mpz_init(temp2);
	mpz_init(temp3);
	mpz_init(candidateY_2);
	mpz_init(pY_2);


	mpz_powm_ui(temp1, P -> x, 3, params -> p);
	mpz_mul(temp2, P -> x, params -> a);
	mpz_add(temp3, temp1, temp2);
	mpz_add(temp1, temp3, params -> b);
	mpz_mod(candidateY_2, temp1, params -> p);

	mpz_powm_ui(pY_2, P -> y, 2, params -> p);


	output = mpz_cmp(candidateY_2, pY_2);


	mpz_clear(temp1);
	mpz_clear(temp2);
	mpz_clear(temp3);
	mpz_clear(candidateY_2);
	mpz_clear(pY_2);

	return output;
}



void freeECC_Params(struct eccParams *params)
{
	mpz_clear(params -> p);
	mpz_clear(params -> a);
	mpz_clear(params -> b);
	mpz_clear(params -> n);
	clearECC_Point(params -> g);

	free(params);
}


unsigned char *hashECC_Point(struct eccPoint *inputPoint, int outputLength)
{
	unsigned char *xBytes, *yBytes, *bytesToHash, *hashedBytes, *outputBytes;
	int tempLengthX = 0, tempLengthY = 0;


	xBytes = convertMPZToBytes(inputPoint -> x, &tempLengthX);
	yBytes = convertMPZToBytes(inputPoint -> y, &tempLengthY);

	bytesToHash = (unsigned char *) calloc(tempLengthX + tempLengthY, sizeof(unsigned char));
	memcpy(bytesToHash, xBytes, tempLengthX);
	memcpy(bytesToHash + tempLengthX, yBytes, tempLengthY);

	hashedBytes = sha256_full(bytesToHash, tempLengthX + tempLengthY);

	outputBytes = (unsigned char *) calloc(outputLength, sizeof(unsigned char));
	memcpy(outputBytes, hashedBytes, outputLength);

	free(xBytes);
	free(yBytes);

	return outputBytes;
}