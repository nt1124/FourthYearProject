struct CRS_ECC *initCRS_ECC()
{
	struct CRS_ECC *crs = (struct CRS_ECC*) calloc(1, sizeof(struct CRS_ECC));

	crs -> g_0 = initECC_Point_Hom();
	crs -> g_1 = initECC_Point_Hom();
	crs -> h_0 = initECC_Point_Hom();
	crs -> h_1 = initECC_Point_Hom();

	return crs;
}


// Converts CRS into bytes and then sends to the provided socket.
int sendCRS_ECC(int writeSocket, int readSocket, struct CRS_ECC *crs)
{
	unsigned char *final_bytes;
	int final_length, g_0_length, g_1_length, h_0_length, h_1_length, offset = 0;


	g_0_length = sizeOfSerial_ECCPoint_Hom(crs -> g_0);
	g_1_length = sizeOfSerial_ECCPoint_Hom(crs -> g_1);
	h_0_length = sizeOfSerial_ECCPoint_Hom(crs -> h_0);
	h_1_length = sizeOfSerial_ECCPoint_Hom(crs -> h_1);
	final_length = g_0_length + g_1_length + h_0_length + h_1_length;

	final_bytes = (unsigned char*) calloc(final_length, sizeof(unsigned char));


	serialise_ECC_Point_Hom(crs -> g_0, final_bytes, &offset);
	serialise_ECC_Point_Hom(crs -> g_1, final_bytes, &offset);
	serialise_ECC_Point_Hom(crs -> h_0, final_bytes, &offset);
	serialise_ECC_Point_Hom(crs -> h_1, final_bytes, &offset);


	sendBoth(writeSocket, (octet*) final_bytes, offset);

	return 1;
}


// Receives the CRS as bytes and creates a CRS from these bytes.
// Probably potential for efficiency gains here if we did the whole CRS in one go.
// Would need to send lengths of each section for that though.
struct CRS_ECC *receiveCRS_ECC(int writeSocket, int readSocket)
{
	struct CRS_ECC *crs = (struct CRS_ECC*) calloc(1, sizeof(struct CRS_ECC));
	unsigned char *curBytes;
	int curLength, offset = 0;

	curBytes = receiveBoth(readSocket, curLength);


	crs -> g_0 = deserialise_ECC_Point_Hom(curBytes, &offset);
	crs -> g_1 = deserialise_ECC_Point_Hom(curBytes, &offset);

	crs -> h_0 = deserialise_ECC_Point_Hom(curBytes, &offset);
	crs -> h_1 = deserialise_ECC_Point_Hom(curBytes, &offset);


	return crs;
}

// Initialise the messy side of the params.
struct messyParams_ECC *initMessyParams_ECC()
{
	struct messyParams_ECC *params = (struct messyParams_ECC*) calloc(1, sizeof(struct messyParams_ECC));

	params -> crs = initCRS_ECC();
	params -> params = initECC_Params_Hom();

	return params;
}

// Ronseal
int sendMessyParams_ECC(int writeSocket, int readSocket, struct messyParams_ECC *paramsToSend)
{
	unsigned char *eccParamsBuffer;
	int bufferLen = 0;

	sendCRS_ECC(writeSocket, readSocket, paramsToSend -> crs);

	//sendDDH_Group(writeSocket, readSocket, paramsToSend -> group);
	eccParamsBuffer = serialiseECC_Params_Hom(paramsToSend -> params, &bufferLen);
	sendBoth(writeSocket, eccParamsBuffer, bufferLen);


	return 1;
}



// Receive the dec params, note that we do NOT get the trapdoor.
struct messyParams_ECC *receiveMessyParams_ECC(int writeSocket, int readSocket)
{
	struct messyParams_ECC *params = (struct messyParams_ECC*) calloc(1, sizeof(struct messyParams_ECC));
	unsigned char *inputBuffer;
	int tempOffset = 0;


	params -> crs = receiveCRS_ECC(writeSocket, readSocket);

	inputBuffer = receiveBoth(readSocket, tempOffset);
	tempOffset = 0;
	params -> params = deserialiseECC_Params_Hom(inputBuffer, &tempOffset);


	return params;
}


// Initialise the dec side of the params
struct decParams_ECC *initDecParams_ECC()
{
	struct decParams_ECC *params = (struct decParams_ECC*) calloc(1, sizeof(struct decParams_ECC));


	params -> crs = initCRS_ECC();
	params -> params = initECC_Params_Hom();


	return params;
}



// Ronseal
int sendDecParams_ECC(int writeSocket, int readSocket, struct decParams_ECC *paramsToSend)
{
	unsigned char *eccParamsBuffer;
	int bufferLen = 0;

	sendCRS_ECC(writeSocket, readSocket, paramsToSend -> crs);

	eccParamsBuffer = serialiseECC_Params_Hom(paramsToSend -> params, &bufferLen);
	sendBoth(writeSocket, eccParamsBuffer, bufferLen);


	return 1;
}


// Receive the dec params, note that we do NOT get the trapdoor.
struct decParams_ECC *receiveDecParams_ECC(int writeSocket, int readSocket)
{
	struct decParams_ECC *params = (struct decParams_ECC*) calloc(1, sizeof(struct decParams_ECC));
	unsigned char *inputBuffer;
	int tempOffset = 0;


	params -> crs = receiveCRS_ECC(writeSocket, readSocket);

	inputBuffer = receiveBoth(readSocket, tempOffset);

	tempOffset = 0;
	params -> params = deserialiseECC_Params_Hom(inputBuffer, &tempOffset);

	return params;
}


// Initialise the key pair.
struct otKeyPair_ECC *initKeyPair_ECC()
{
	struct otKeyPair_ECC *keyPair = (struct otKeyPair_ECC*) calloc(1, sizeof(struct otKeyPair_ECC));
	
	keyPair -> pk = (PVM_OT_PK_ECC*) calloc(1, sizeof(PVM_OT_PK_ECC));
	keyPair -> pk -> g = initECC_Point_Hom();
	keyPair -> pk -> h = initECC_Point_Hom();

	mpz_init( keyPair -> sk );

	return keyPair;
}


unsigned char *serialise_PKs_otKeyPair_ECC_Array(struct otKeyPair_ECC **inputs, int numKeys, int *outputLength)
{
	unsigned char *outputBuffer;
	int totalLength = 0;
	int i, bufferOffset = 0;


	for(i = 0; i < numKeys; i ++)
	{
		totalLength += sizeOfSerial_ECCPoint_Hom(inputs[i] -> pk -> g);
		totalLength += sizeOfSerial_ECCPoint_Hom(inputs[i] -> pk -> h);
	}

	outputBuffer = (unsigned char *) calloc(totalLength, sizeof(unsigned char));

	for(i = 0; i < numKeys; i ++)
	{
		serialise_ECC_Point_Hom(inputs[i] -> pk -> g, outputBuffer, &bufferOffset);
		serialise_ECC_Point_Hom(inputs[i] -> pk -> h, outputBuffer, &bufferOffset);
	}

	*outputLength = totalLength;

	return outputBuffer;
}


struct otKeyPair_ECC **deserialise_PKs_otKeyPair_ECC_Array(unsigned char *inputBuffer, int numKeys)
{
	struct otKeyPair_ECC **outputKeys = (struct otKeyPair_ECC **) calloc(numKeys, sizeof(struct otKeyPair_ECC*));
	int i, bufferOffset = 0;


	for(i = 0; i < numKeys; i ++)
	{
		outputKeys[i] = initKeyPair_ECC();

		outputKeys[i] -> pk -> g = deserialise_ECC_Point_Hom(inputBuffer, &bufferOffset);		
		outputKeys[i] -> pk -> h = deserialise_ECC_Point_Hom(inputBuffer, &bufferOffset);
	}

	return outputKeys;
}



int sizeOfSerial_ECC_U_V_Pair(struct u_v_Pair_ECC *input)
{
	int sizeNeeded = 0;

	sizeNeeded += sizeOfSerial_ECCPoint(input -> u);
	sizeNeeded += sizeOfSerial_ECCPoint(input -> v);

	return sizeNeeded;
}


unsigned char *serialise_U_V_Pair_Array_ECC(struct u_v_Pair_ECC **inputs, int num_u_v_Pairs, int *outputLength)
{
	unsigned char *outputBuffer;
	int totalLength = 0;
	int i, bufferOffset = 0;


	for(i = 0; i < num_u_v_Pairs; i ++)
	{
		totalLength += sizeOfSerial_ECC_U_V_Pair(inputs[i]);
	}

	outputBuffer = (unsigned char *) calloc(totalLength, sizeof(unsigned char));

	for(i = 0; i < num_u_v_Pairs; i ++)
	{
		serialise_U_V_pair_ECC(inputs[i], outputBuffer, &bufferOffset);
	}

	*outputLength = totalLength;

	return outputBuffer;
}


struct u_v_Pair_ECC **deserialise_U_V_Pair_Array_ECC(unsigned char *inputBuffer, int num_u_v_Pairs)
{
	struct u_v_Pair_ECC **outputKeys = (struct u_v_Pair_ECC **) calloc(num_u_v_Pairs, sizeof(struct u_v_Pair_ECC*));
	mpz_t *tempMPZ;
	int i, bufferOffset = 0;


	for(i = 0; i < num_u_v_Pairs; i ++)
	{
		outputKeys[i] = deserialise_U_V_pair_ECC(inputBuffer, &bufferOffset);
	}

	return outputKeys;
}