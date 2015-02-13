struct CRS *initCRS()
{
	struct CRS *crs = (struct CRS*) calloc(1, sizeof(struct CRS));

	mpz_init(crs -> g_0);
	mpz_init(crs -> g_1);
	mpz_init(crs -> h_0);
	mpz_init(crs -> h_1);

	return crs;
}


// Converts CRS into bytes and then sends to the provided socket.
int sendCRS(int writeSocket, int readSocket, struct CRS *crs)
{
	unsigned char *final_bytes;
	int final_length, g_0_length, g_1_length, h_0_length, h_1_length, offset = 0;

	g_0_length = sizeof(mp_limb_t) * mpz_size(crs -> g_0);
	g_1_length = sizeof(mp_limb_t) * mpz_size(crs -> g_1);
	h_0_length = sizeof(mp_limb_t) * mpz_size(crs -> h_0);
	h_1_length = sizeof(mp_limb_t) * mpz_size(crs -> h_1);
	final_length = g_0_length + g_1_length + h_0_length + h_1_length;

	final_bytes = (unsigned char*) calloc(final_length, sizeof(unsigned char));

	serialiseMPZ(crs -> g_0, final_bytes, &offset);
	serialiseMPZ(crs -> g_1, final_bytes, &offset);
	serialiseMPZ(crs -> h_0, final_bytes, &offset);
	serialiseMPZ(crs -> h_1, final_bytes, &offset);


	sendBoth(writeSocket, (octet*) final_bytes, offset);

	return 1;
}


// Receives the CRS as bytes and creates a CRS from these bytes.
// Probably potential for efficiency gains here if we did the whole CRS in one go.
// Would need to send lengths of each section for that though.
struct CRS *receiveCRS(int writeSocket, int readSocket)
{
	struct CRS *crs = initCRS();
	unsigned char *curBytes;
	int curLength, offset = 0;
	mpz_t *tempMPZ;


	curBytes = receiveBoth(readSocket, curLength);

	tempMPZ = deserialiseMPZ(curBytes, &offset);
	mpz_set(crs -> g_0, *tempMPZ);
	free(tempMPZ);

	tempMPZ = deserialiseMPZ(curBytes, &offset);
	mpz_set(crs -> g_1, *tempMPZ);
	free(tempMPZ);

	tempMPZ = deserialiseMPZ(curBytes, &offset);
	mpz_set(crs -> h_0, *tempMPZ);
	free(tempMPZ);

	tempMPZ = deserialiseMPZ(curBytes, &offset);
	mpz_set(crs -> h_1, *tempMPZ);
	free(tempMPZ);


	return crs;
}


// Initialise the messy side of the trapdoor.
struct TrapdoorMessy *initTrapdoorMessy()
{
	struct TrapdoorMessy *t = (struct TrapdoorMessy*) calloc(1, sizeof(struct TrapdoorMessy));

	mpz_init(t -> x_0);
	mpz_init(t -> x_1);

	return t;
}


// Initialise the dec side of the trapdoor.
struct TrapdoorDecKey *initTrapdoorDecKey()
{
	struct TrapdoorDecKey *tDecKey = (struct TrapdoorDecKey*) calloc(1, sizeof(struct TrapdoorDecKey));

	tDecKey -> pk = (struct PVM_OT_PK*) calloc(1, sizeof(struct PVM_OT_PK));
	mpz_init(tDecKey -> r);
	mpz_init(tDecKey -> r_yInv);

	return tDecKey;
}


// Initialise the messy side of the params.
struct messyParams *initMessyParams()
{
	struct messyParams *params = (struct messyParams*) calloc(1, sizeof(struct messyParams));

	params -> crs = initCRS();
	params -> group = initGroupStruct();
	params -> trapdoor = initTrapdoorMessy();

	return params;
}


// Ronseal
int sendMessyParams(int writeSocket, int readSocket, struct messyParams *paramsToSend)
{

	sendCRS(writeSocket, readSocket, paramsToSend -> crs);

	sendDDH_Group(writeSocket, readSocket, paramsToSend -> group);

	return 1;
}


// Receive the dec params, note that we do NOT get the trapdoor.
struct messyParams *receiveMessyParams(int writeSocket, int readSocket)
{
	struct messyParams *params = (struct messyParams*) calloc(1, sizeof(struct decParams));

	params -> crs = receiveCRS(writeSocket, readSocket);

	params -> group = receiveDDH_Group(writeSocket, readSocket);

	return params;
}

// Initialise the dec side of the params
struct decParams *initDecParams()
{
	struct decParams *params = (struct decParams*) calloc(1, sizeof(struct messyParams));

	params -> crs = initCRS();
	params -> group = initGroupStruct();
	params -> trapdoor = (TrapdoorDec*) calloc(1, sizeof(TrapdoorDec));
	mpz_init( *(params -> trapdoor) );

	return params;
}


// Ronseal
int sendDecParams(int writeSocket, int readSocket, struct decParams *paramsToSend)
{

	sendCRS(writeSocket, readSocket, paramsToSend -> crs);

	sendDDH_Group(writeSocket, readSocket, paramsToSend -> group);

	return 1;
}


// Receive the dec params, note that we do NOT get the trapdoor.
struct decParams *receiveDecParams(int writeSocket, int readSocket)
{
	struct decParams *params = (struct decParams*) calloc(1, sizeof(struct decParams));

	params -> crs = receiveCRS(writeSocket, readSocket);

	params -> group = receiveDDH_Group(writeSocket, readSocket);

	return params;
}


// Initialise the key pair.
struct otKeyPair *initKeyPair()
{
	struct otKeyPair *keyPair = (struct otKeyPair*) calloc(1, sizeof(struct otKeyPair));
	
	keyPair -> pk = (PVM_OT_PK*) calloc(1, sizeof(PVM_OT_PK));
	mpz_init(keyPair -> pk -> g);
	mpz_init(keyPair -> pk -> h);

	keyPair -> sk = (PVM_OT_SK*) calloc(1, sizeof(PVM_OT_SK));
	mpz_init( *(keyPair -> sk) );

	return keyPair;
}

	
void serialise_U_V_pair(struct u_v_Pair *c, 
						unsigned char *outputBuffer, int *bufferOffset)
{
	int outputOffset = *bufferOffset;


	serialiseMPZ(c -> u, outputBuffer, &outputOffset);
	serialiseMPZ(c -> v, outputBuffer, &outputOffset);


	*bufferOffset = outputOffset;
}


struct u_v_Pair *deserialise_U_V_pair(unsigned char *inputBuffer, int *bufferOffset)
{
	mpz_t *tempMPZ;
	struct u_v_Pair *c = init_U_V();
	int tempOffset = *bufferOffset;


	tempMPZ = deserialiseMPZ(inputBuffer, &tempOffset);
	mpz_set(c -> u, *tempMPZ);
	free(tempMPZ);
	
	tempMPZ = deserialiseMPZ(inputBuffer, &tempOffset);
	mpz_set(c -> v, *tempMPZ);
	free(tempMPZ);


	*bufferOffset = tempOffset;

	return c;
}


unsigned char *serialise_PKs_otKeyPair_Array(struct otKeyPair **inputs, int numKeys, int *outputLength)
{
	unsigned char *outputBuffer;
	int totalLength = 2 * sizeof(int) * numKeys;
	int i, bufferOffset = 0;


	for(i = 0; i < numKeys; i ++)
	{
		totalLength += ( sizeof(mp_limb_t) * mpz_size(inputs[i] -> pk -> g) );
		totalLength += ( sizeof(mp_limb_t) * mpz_size(inputs[i] -> pk -> h) );
	}

	outputBuffer = (unsigned char *) calloc(totalLength, sizeof(unsigned char));

	for(i = 0; i < numKeys; i ++)
	{
		serialiseMPZ(inputs[i] -> pk -> g, outputBuffer, &bufferOffset);
		serialiseMPZ(inputs[i] -> pk -> h, outputBuffer, &bufferOffset);
	}

	*outputLength = totalLength;

	return outputBuffer;
}


struct otKeyPair **deserialise_PKs_otKeyPair_Array(unsigned char *inputBuffer, int numKeys)
{
	struct otKeyPair **outputKeys = (struct otKeyPair **) calloc(numKeys, sizeof(struct otKeyPair*));
	mpz_t *tempMPZ;
	int i, bufferOffset = 0;


	for(i = 0; i < numKeys; i ++)
	{
		outputKeys[i] = initKeyPair();

		tempMPZ = deserialiseMPZ(inputBuffer, &bufferOffset);
		mpz_set(outputKeys[i] -> pk -> g, *tempMPZ);
		free(tempMPZ);
		
		tempMPZ = deserialiseMPZ(inputBuffer, &bufferOffset);
		mpz_set(outputKeys[i] -> pk -> h, *tempMPZ);
		free(tempMPZ);
	}

	return outputKeys;
}



unsigned char *serialise_U_V_Pair_Array(struct u_v_Pair **inputs, int num_u_v_Pairs, int *outputLength)
{
	unsigned char *outputBuffer;
	int totalLength = 2 * sizeof(int) * num_u_v_Pairs;
	int i, bufferOffset = 0;


	for(i = 0; i < num_u_v_Pairs; i ++)
	{
		totalLength += ( sizeof(mp_limb_t) * mpz_size(inputs[i] -> u) );
		totalLength += ( sizeof(mp_limb_t) * mpz_size(inputs[i] -> v) );
	}

	outputBuffer = (unsigned char *) calloc(totalLength, sizeof(unsigned char));

	for(i = 0; i < num_u_v_Pairs; i ++)
	{
		serialise_U_V_pair(inputs[i], outputBuffer, &bufferOffset);
	}

	*outputLength = totalLength;

	return outputBuffer;
}


struct u_v_Pair **deserialise_U_V_Pair_Array(unsigned char *inputBuffer, int num_u_v_Pairs)
{
	struct u_v_Pair **outputKeys = (struct u_v_Pair **) calloc(num_u_v_Pairs, sizeof(struct u_v_Pair*));
	mpz_t *tempMPZ;
	int i, bufferOffset = 0;


	for(i = 0; i < num_u_v_Pairs; i ++)
	{
		outputKeys[i] = deserialise_U_V_pair(inputBuffer, &bufferOffset);
	}

	return outputKeys;
}