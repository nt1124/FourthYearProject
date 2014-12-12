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
	unsigned char *curBytes;
	int curLength;

	curBytes = convertMPZToBytes( crs -> g_0, &curLength);
	sendBoth(writeSocket, (octet*) curBytes, curLength);

	curBytes = convertMPZToBytes( crs -> h_0, &curLength);
	sendBoth(writeSocket, (octet*) curBytes, curLength);

	curBytes = convertMPZToBytes( crs -> g_1, &curLength);
	sendBoth(writeSocket, (octet*) curBytes, curLength);

	curBytes = convertMPZToBytes( crs -> h_1, &curLength);
	sendBoth(writeSocket, (octet*) curBytes, curLength);

	return 1;
}


// Receives the CRS as bytes and creates a CRS from these bytes.
// Probably potential for efficiency gains here if we did the whole CRS in one go.
// Would need to send lengths of each section for that though.
struct CRS *receiveCRS(int writeSocket, int readSocket)
{
	struct CRS *crs = initCRS();
	unsigned char *curBytes;
	int curLength;
	
	mpz_t *tempMPZ = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_init(*tempMPZ);


	curBytes = receiveBoth(readSocket, curLength);
	convertBytesToMPZ(tempMPZ, curBytes, curLength);
	mpz_set(crs -> g_0, *tempMPZ);

	curBytes = receiveBoth(readSocket, curLength);
	convertBytesToMPZ(tempMPZ, curBytes, curLength);
	mpz_set(crs -> h_0, *tempMPZ);

	curBytes = receiveBoth(readSocket, curLength);
	convertBytesToMPZ(tempMPZ, curBytes, curLength);
	mpz_set(crs -> g_1, *tempMPZ);

	curBytes = receiveBoth(readSocket, curLength);
	convertBytesToMPZ(tempMPZ, curBytes, curLength);
	mpz_set(crs -> h_1, *tempMPZ);

	free(tempMPZ);

	return crs;
}


struct TrapdoorMessy *initTrapdoorMessy()
{
	struct TrapdoorMessy *t = (struct TrapdoorMessy*) calloc(1, sizeof(struct TrapdoorMessy));

	mpz_init(t -> x_0);
	mpz_init(t -> x_1);

	return t;
}


struct TrapdoorDecKey *initTrapdoorDecKey()
{
	struct TrapdoorDecKey *tDecKey = (struct TrapdoorDecKey*) calloc(1, sizeof(struct TrapdoorDecKey));

	tDecKey -> pk = (struct PVM_OT_PK*) calloc(1, sizeof(struct PVM_OT_PK));
	mpz_init(tDecKey -> r);
	mpz_init(tDecKey -> r_yInv);

	return tDecKey;
}


struct messyParams *initMessyParams()
{
	struct messyParams *params = (struct messyParams*) calloc(1, sizeof(struct messyParams));

	params -> crs = initCRS();
	params -> group = initGroupStruct();
	params -> trapdoor = initTrapdoorMessy();

	return params;
}


struct decParams *initDecParams()
{
	struct decParams *params = (struct decParams*) calloc(1, sizeof(struct messyParams));

	params -> crs = initCRS();
	params -> group = initGroupStruct();
	params -> trapdoor = (TrapdoorDec*) calloc(1, sizeof(TrapdoorDec));
	mpz_init( *(params -> trapdoor) );

	return params;
}


// Whoops, pretty sure we shouldn't be sending the Trapdoor value...
int sendDecParams(int writeSocket, int readSocket, struct decParams *paramsToSend)
{
	/*
	unsigned char *trapdoorBytes;
	int trapdoorBytesLen;
	*/
	sendCRS(writeSocket, readSocket, paramsToSend -> crs);

	sendDDH_Group(writeSocket, readSocket, paramsToSend -> group);

	/*
	trapdoorBytes = convertMPZToBytes( *(paramsToSend -> trapdoor), &trapdoorBytesLen);
	sendBoth(writeSocket, (octet*) trapdoorBytes, trapdoorBytesLen);
	*/
	return 1;
}



struct decParams *receiveDecParams(int writeSocket, int readSocket)
{
	struct decParams *params = (struct decParams*) calloc(1, sizeof(struct decParams));

	params -> crs = receiveCRS(writeSocket, readSocket);

	params -> group = receiveDDH_Group(writeSocket, readSocket);

	return params;
}


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




// Generates a group 
struct messyParams *setupMessy(int securityParam, gmp_randstate_t state)
{
	struct messyParams *params = initMessyParams();

	params -> group = generateGroup(securityParam, state);

	do
	{
		mpz_urandomm(params -> crs -> g_0, state, params -> group -> p);
	} while( 0 < mpz_cmp_ui(params -> crs -> g_0, 1) );

	do
	{
		mpz_urandomm(params -> crs -> g_1, state, params -> group -> p);
	} while( 0 < mpz_cmp_ui(params -> crs -> g_1, 1) );
	
	do
	{
		mpz_urandomm(params -> trapdoor -> x_0, state, params -> group -> p);
	} while( 0 != mpz_cmp_ui(params -> trapdoor -> x_0, 0) );

	do
	{
		mpz_urandomm(params -> trapdoor -> x_1, state, params -> group -> p);
	} while( 0 != mpz_cmp_ui(params -> trapdoor -> x_1, 0) &&
			 0 == mpz_cmp(params -> trapdoor -> x_0, params -> trapdoor -> x_1) );

	mpz_powm(params -> crs -> h_0, params -> crs -> g_0, params -> trapdoor -> x_0, params -> group -> p);
	mpz_powm(params -> crs -> h_1, params -> crs -> g_1, params -> trapdoor -> x_1, params -> group -> p);

	return params;
}


struct decParams *setupDec(int securityParam, gmp_randstate_t state)
{
	mpz_t x, y;
	struct decParams *params = initDecParams();
	params -> group = generateGroup(securityParam, state);

	mpz_init(x);
	mpz_init(y);

	do
	{
		mpz_urandomm(params -> crs -> g_0, state, params -> group -> p);
	} while( 0 > mpz_cmp_ui(params -> crs -> g_0, 1) );

	do
	{
		mpz_urandomm(x, state, params -> group -> p);
	} while( 0 > mpz_cmp_ui(x, 1) );

	do
	{
		mpz_urandomm(*(params -> trapdoor), state, params -> group -> p);
	} while( 0 == mpz_cmp_ui( *(params -> trapdoor), 0) );


	mpz_powm(params -> crs -> g_1, params -> crs -> g_0, *(params -> trapdoor), params -> group -> p);
	mpz_powm(params -> crs -> h_0, params -> crs -> g_0, x, params -> group -> p);
	mpz_powm(params -> crs -> h_1, params -> crs -> g_1, x, params -> group -> p);


	return params;
}


struct otKeyPair *keyGen(struct CRS *crs, unsigned char sigmaBit,
						struct DDH_Group *group, gmp_randstate_t state)
{
	struct otKeyPair *keyPair = initKeyPair();

	do
	{
		mpz_urandomm( *(keyPair -> sk), state, group -> p);
	} while( 0 == mpz_cmp_ui(*(keyPair -> sk), 0) );

	// CRS is null here. Need to use those structs for messyparams etc.
	// Potential to change crs to {g[2], h[2]} and then avoid branching.
	if(0 == sigmaBit)
	{
		mpz_powm(keyPair -> pk -> g, crs -> g_0, *(keyPair -> sk), group -> p);
		mpz_powm(keyPair -> pk -> h, crs -> h_0, *(keyPair -> sk), group -> p);
	}
	else if(1 == sigmaBit)
	{
		mpz_powm(keyPair -> pk -> g, crs -> g_1, *(keyPair -> sk), group -> p);
		mpz_powm(keyPair -> pk -> h, crs -> h_1, *(keyPair -> sk), group -> p);
	}

	return keyPair;
}


struct DDH_PK *setPrimitivePK(struct CRS *crs,
							struct PVM_OT_PK *otPK, int sigmaBit)
{
	struct DDH_PK *pk = initPublicKey();


	mpz_set(pk -> g_x, otPK -> g);
	mpz_set(pk -> h_x, otPK -> h);

	if(0 == sigmaBit)
	{
		mpz_set(pk -> g, crs -> g_0);
		mpz_set(pk -> h, crs -> h_0);
	}
	else
	{
		mpz_set(pk -> g, crs -> g_1);
		mpz_set(pk -> h, crs -> h_1);
	}

	return pk;
}


// Could we speed up by passing in the DDH_PK Blue Peter-ed?
struct u_v_Pair *PVW_OT_Enc(mpz_t M, 
							struct CRS *crs, struct DDH_Group *group, gmp_randstate_t state,
							struct PVM_OT_PK *otPK, unsigned char sigmaBit)
{
	struct u_v_Pair *CT = init_U_V();
	struct DDH_PK *pk = setPrimitivePK(crs, otPK, sigmaBit);

	CT = encDDH(pk, group, M, state);

	return CT;
}


mpz_t *PVW_OT_Dec(struct u_v_Pair *CT,
				struct CRS *crs, struct DDH_Group *group,
				PVM_OT_SK *sk)
{
	mpz_t *M_Prime = decDDH(sk, group, CT);
	
	return M_Prime;
}


unsigned char findMessy(struct CRS *crs, struct DDH_Group *group,
						struct TrapdoorMessy *t, struct PVM_OT_PK *pk)
{
	mpz_t g_x_0;

	if( 0 == mpz_cmp(t -> x_0, t -> x_1) )
	{
		printf("ERROR: x_0 == x_1 in Trapdoor for OT_PVW_DDH\n");
		return 0xFF;
	}

	mpz_init(g_x_0);
	mpz_powm(g_x_0, pk -> g, t -> x_0, group -> p);

	if( 0 != mpz_cmp(g_x_0, pk -> h) )
	{
		return 0x00;
	}

	return 0x01;
}


struct TrapdoorDecKey *trapdoorKeyGeneration( struct CRS *crs, struct DDH_Group *group,
											gmp_randstate_t state, TrapdoorDec *t)
{
	struct TrapdoorDecKey *tDecKey = initTrapdoorDecKey();
	mpz_t yInv;

	mpz_init(yInv);

	mpz_urandomm(tDecKey -> r, state, group -> p);

	mpz_powm(tDecKey -> pk -> g, crs -> g_0, tDecKey -> r, group -> p);
	mpz_powm(tDecKey -> pk -> h, crs -> h_0, tDecKey -> r, group -> p);

	mpz_invert(yInv, *t, group -> p);

	mpz_mul(tDecKey -> r_yInv, tDecKey -> r, yInv);
	mpz_mod(tDecKey -> r_yInv, tDecKey -> r_yInv, group -> p);

	return tDecKey;
}



// All the infrastructre is now complete, now we use what we have to perform OT.


// Request a set of params from the Receiver who will run setupDec.
// Note that the sender will NOT be in possession of the TrapdoorDec.
struct decParams *senderCRS_Syn(int writeSocket, int readSocket)
{
	struct decParams *params = receiveDecParams(writeSocket, readSocket);

	return params;
}


void senderOT_UC(int writeSocket, int readSocket,
				unsigned char *input0Bytes, unsigned char *input1Bytes, int inputLengths,
				struct decParams *params, gmp_randstate_t *state)
{
	struct otKeyPair *keyPair = initKeyPair();
	struct u_v_Pair *c_0, *c_1;
	mpz_t *outputMPZ, *tempMPZ = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_t *input0 = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_t *input1 = (mpz_t*) calloc(1, sizeof(mpz_t));
	unsigned char *outputBytes, *curBytes;
	int curLength;

	mpz_init(*tempMPZ);
	mpz_init(*input0);
	mpz_init(*input1);
	

	curBytes = receiveBoth(readSocket, curLength);
	convertBytesToMPZ(tempMPZ, curBytes, curLength);
	mpz_set(keyPair -> pk -> g, *tempMPZ);

	curBytes = receiveBoth(readSocket, curLength);
	convertBytesToMPZ(tempMPZ, curBytes, curLength);
	mpz_set(keyPair -> pk -> h, *tempMPZ);

	convertBytesToMPZ(input0, input0Bytes, inputLengths);
	convertBytesToMPZ(input1, input1Bytes, inputLengths);

	c_0 = PVW_OT_Enc(*input0, params -> crs, params -> group, *state, keyPair -> pk, 0x00);
	c_1 = PVW_OT_Enc(*input1, params -> crs, params -> group, *state, keyPair -> pk, 0x01);

	curBytes = convertMPZToBytes(c_0 -> u, &curLength);
	sendBoth(writeSocket, (octet*) curBytes, curLength);
	curBytes = convertMPZToBytes(c_0 -> v, &curLength);
	sendBoth(writeSocket, (octet*) curBytes, curLength);

	curBytes = convertMPZToBytes(c_1 -> u, &curLength);
	sendBoth(writeSocket, (octet*) curBytes, curLength);
	curBytes = convertMPZToBytes(c_1 -> v, &curLength);
	sendBoth(writeSocket, (octet*) curBytes, curLength);

	free(tempMPZ);
}


struct decParams *receiverCRS_Syn(int writeSocket, int readSocket, int securityParam, gmp_randstate_t state)
{
	struct decParams *params = setupDec( securityParam, state );

	sendDecParams(writeSocket, readSocket, params);

	return params;
}


unsigned char *receiverOT_UC(int writeSocket, int readSocket,
							unsigned char inputBit, struct decParams *params,
							int *outputLength, gmp_randstate_t *state)
{
	struct otKeyPair *keyPair = keyGen(params -> crs, inputBit, params -> group, *state);
	struct u_v_Pair *c_0 = init_U_V(), *c_1 = init_U_V();
	mpz_t *outputMPZ, *tempMPZ = (mpz_t*) calloc(1, sizeof(mpz_t));
	unsigned char *outputBytes, *curBytes;
	int curLength;
	
	mpz_init(*tempMPZ);


	curBytes = convertMPZToBytes(keyPair -> pk -> g, &curLength);
	sendBoth(writeSocket, (octet*) curBytes, curLength);
	curBytes = convertMPZToBytes(keyPair -> pk -> h, &curLength);
	sendBoth(writeSocket, (octet*) curBytes, curLength);

	curBytes = receiveBoth(readSocket, curLength);
	convertBytesToMPZ(tempMPZ, curBytes, curLength);
	mpz_set(c_0 -> u, *tempMPZ);

	curBytes = receiveBoth(readSocket, curLength);
	convertBytesToMPZ(tempMPZ, curBytes, curLength);
	mpz_set(c_0 -> v, *tempMPZ);


	curBytes = receiveBoth(readSocket, curLength);
	convertBytesToMPZ(tempMPZ, curBytes, curLength);
	mpz_set(c_1 -> u, *tempMPZ);

	curBytes = receiveBoth(readSocket, curLength);
	convertBytesToMPZ(tempMPZ, curBytes, curLength);
	mpz_set(c_1 -> v, *tempMPZ);

	if(0x00 == inputBit)
		outputMPZ = PVW_OT_Dec(c_0, params -> crs, params -> group, keyPair -> sk);
	else
		outputMPZ = PVW_OT_Dec(c_1, params -> crs, params -> group, keyPair -> sk);

	free(tempMPZ);

	outputBytes = convertMPZToBytes(*outputMPZ, outputLength);

	return outputBytes;
}



int testOT_PWV_DDH_Local(int receiverOrSender)
{
	mpz_t *input0 = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_t *input1 = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_t *decSigma = (mpz_t*) calloc(1, sizeof(mpz_t));
	struct u_v_Pair *y0, *y1;
	struct otKeyPair *keyPair;
	
	unsigned char *input0Bytes = generateRandBytes(16, 16);
	unsigned char *input1Bytes = generateRandBytes(16, 16);
	unsigned char *outputBytes1, *outputBytes2;
	int tempInt1, tempInt2;

	gmp_randstate_t *state = seedRandGen();
	struct decParams *params;

	params = setupDec( 1024, *state );

	convertBytesToMPZ(input0, input0Bytes, 16);
	convertBytesToMPZ(input1, input1Bytes, 16);

	keyPair = keyGen(params -> crs, 0x00, params -> group, *state);

	y0 = PVW_OT_Enc(*input0, params -> crs, params -> group, *state, keyPair -> pk, 0x00);
	y1 = PVW_OT_Enc(*input1, params -> crs, params -> group, *state, keyPair -> pk, 0x01);

	decSigma = PVW_OT_Dec(y0, params -> crs, params -> group, keyPair -> sk);

	outputBytes1 = convertMPZToBytes(*decSigma, &tempInt1);
	outputBytes2 = convertMPZToBytes(*input0, &tempInt2);

	if(16 == tempInt1)
		if(strncmp((char*)outputBytes1, (char*)input0Bytes, tempInt1) == 0)
			return 1;

	int i;
	printf("%d = %d\n", 16, tempInt1);
	for(i = 0; i < tempInt1; i ++)
	{	
		printf("%u + %u + %u\n", input0Bytes[i], outputBytes1[i], outputBytes2[i]);
	}

	return 0;
}


void testSender_OT_PVW()
{
	struct sockaddr_in destWrite, destRead;
	int writeSocket, readSocket, mainWriteSock, mainReadSock;
	int writePort, readPort, i;
	gmp_randstate_t *state = seedRandGen();

	writePort = 7654;
	readPort = writePort + 1;

	unsigned char *input0Bytes = generateRandBytes(16, 16);
	unsigned char *input1Bytes = generateRandBytes(16, 16);
	struct decParams *params;

	set_up_server_socket(destWrite, writeSocket, mainWriteSock, writePort);
	set_up_server_socket(destRead, readSocket, mainReadSock, readPort);

	params = senderCRS_Syn(writeSocket, readSocket); 
	senderOT_UC(writeSocket, readSocket, input0Bytes, input1Bytes, 16, params, state);

	close_server_socket(writeSocket, mainWriteSock);
	close_server_socket(readSocket, mainReadSock);
}


void testReceive_OT_PVW(char *ipAddress)
{
	int writeSocket, readSocket, writePort, readPort;
	struct sockaddr_in serv_addr_write;
	struct sockaddr_in serv_addr_read;

	readPort = 7654;
	writePort = readPort + 1;
	unsigned char inputBit = 0x01;
	unsigned char *output;
	struct decParams *params;
	int outputLength;
	int i;

	gmp_randstate_t *state = seedRandGen();

    set_up_client_socket(readSocket, ipAddress, readPort, serv_addr_read);
    set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr_write);


	params = receiverCRS_Syn(writeSocket, readSocket, 1024, *state);

	output = receiverOT_UC(writeSocket, readSocket, inputBit, params, &outputLength, state);

	close_client_socket(readSocket);
	close_client_socket(writeSocket);
}