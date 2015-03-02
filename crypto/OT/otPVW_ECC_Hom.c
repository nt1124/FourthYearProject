// Generates a group 
struct messyParams_ECC *setupMessy_ECC(int securityParam, gmp_randstate_t state)
{
	struct messyParams_ECC *params = initMessyParams_ECC();
	mpz_t tempMPZ, x_0, x_1;

	params -> params = initBrainpool_256_Curve_Hom();

	mpz_init(tempMPZ);
	mpz_init(x_0);
	mpz_init(x_1);

	do
	{
		mpz_urandomm(tempMPZ, state, params -> params -> n);
	} while( 0 < mpz_cmp_ui(tempMPZ, 1) );
	params -> crs -> g_0 = windowedScalarPoint_Hom(tempMPZ, params -> params -> g, params -> params);

	do
	{
		mpz_urandomm(tempMPZ, state, params -> params -> n);
	} while( 0 < mpz_cmp_ui(tempMPZ, 1) );
	params -> crs -> g_1 = windowedScalarPoint_Hom(tempMPZ, params -> params -> g, params -> params);
	
	do
	{
		mpz_urandomm(x_0, state, params -> params -> n);
	} while( 0 != mpz_cmp_ui(x_0, 0) );

	do
	{
		mpz_urandomm(x_1, state, params -> params -> n);
	} while( 0 != mpz_cmp_ui(x_1, 0) &&
			 0 == mpz_cmp(x_0, x_1) );

	params -> crs -> h_0 = windowedScalarPoint_Hom(x_0, params -> params -> g, params -> params);
	params -> crs -> h_1 = windowedScalarPoint_Hom(x_1, params -> params -> g, params -> params);


	mpz_clear(x_0);
	mpz_clear(x_1);
	mpz_clear(tempMPZ);


	return params;
}


struct decParams_ECC *setupDec_ECC(int securityParam, gmp_randstate_t state)
{
	mpz_t x, y, tempMPZ;
	struct decParams_ECC *params = initDecParams_ECC();

	params -> params = initBrainpool_256_Curve_Hom();

	mpz_init(x);
	mpz_init(y);
	mpz_init(tempMPZ);


	do
	{
		mpz_urandomm(tempMPZ, state, params -> params -> n);
	} while( 0 == mpz_cmp_ui(tempMPZ, 0) );
	params -> crs -> g_0 = windowedScalarPoint_Hom(tempMPZ, params -> params -> g, params -> params);

	do
	{
		mpz_urandomm(x, state, params -> params -> n);
	} while( 0 == mpz_cmp_ui(x, 0) );

	do
	{
		mpz_urandomm(y, state, params -> params -> n);
	} while( 0 == mpz_cmp_ui( y, 0) );

	params -> crs -> g_1 = windowedScalarPoint_Hom(y, params -> crs -> g_0, params -> params);
	params -> crs -> h_0 = windowedScalarPoint_Hom(x, params -> crs -> g_0, params -> params);
	params -> crs -> h_1 = windowedScalarPoint_Hom(x, params -> crs -> g_1, params -> params);


	mpz_clear(x);
	mpz_clear(y);
	mpz_clear(tempMPZ);

	return params;
}


struct otKeyPair_ECC *keyGen_ECC(struct CRS_ECC *crs, unsigned char sigmaBit,
						struct eccParams_Hom *params, gmp_randstate_t state)
{
	struct otKeyPair_ECC *keyPair = initKeyPair_ECC();

	do
	{
		mpz_urandomm( keyPair -> sk, state, params -> n);
	} while( 0 == mpz_cmp_ui(keyPair -> sk, 0) );

	// CRS is null here. Need to use those structs for messyparams etc.
	// Potential to change crs to {g[2], h[2]} and then avoid branching.
	if(0 == sigmaBit)
	{
		keyPair -> pk -> g = windowedScalarPoint_Hom(keyPair -> sk, crs -> g_0, params);
		keyPair -> pk -> h = windowedScalarPoint_Hom(keyPair -> sk, crs -> h_0, params);
	}
	else if(1 == sigmaBit)
	{
		keyPair -> pk -> g = windowedScalarPoint_Hom(keyPair -> sk, crs -> g_1, params);
		keyPair -> pk -> h = windowedScalarPoint_Hom(keyPair -> sk, crs -> h_1, params);
	}

	return keyPair;
}


struct ECC_PK_Hom *setPrimitivePK_ECC(struct CRS_ECC *crs,
								struct PVM_OT_PK_ECC *otPK, int sigmaBit)
{
	struct ECC_PK_Hom *pk = (struct ECC_PK_Hom*) calloc(1, sizeof(struct ECC_PK_Hom));



	pk -> g_x = copyECC_Point_Hom(otPK -> g);
	pk -> h_x = copyECC_Point_Hom(otPK -> h);

	if(0 == sigmaBit)
	{
		pk -> g = copyECC_Point_Hom(crs -> g_0);
		pk -> h = copyECC_Point_Hom(crs -> h_0);
	}
	else
	{
		pk -> g = copyECC_Point_Hom(crs -> g_1);
		pk -> h = copyECC_Point_Hom(crs -> h_1);
	}

	return pk;
}


// Could we speed up by passing in the DDH_PK Blue Peter-ed?
struct u_v_Pair_ECC_Hom *PVW_OT_Enc_ECC(mpz_t M, 
							struct CRS_ECC *crs, struct eccParams_Hom *params, gmp_randstate_t state,
							struct PVM_OT_PK_ECC *otPK, unsigned char sigmaBit)
{
	struct u_v_Pair_ECC_Hom *CT;
	struct ECC_PK_Hom *pk = setPrimitivePK_ECC(crs, otPK, sigmaBit);
	struct eccPoint_Hom *msgPoint = mapMPZ_To_Point_Hom(M, params);

	CT = ECC_Enc_Hom(pk, msgPoint, params, state);

	return CT;
}


mpz_t *PVW_OT_Dec_ECC(struct u_v_Pair_ECC_Hom *CT,
				struct CRS_ECC *crs, struct eccParams_Hom *params, mpz_t sk)
{
	mpz_t *M_Prime;
	struct eccPoint_Hom *M_Point = ECC_Dec_Hom(sk, CT, params);
	
	M_Prime = mapPoint_Hom_To_MPZ(M_Point, params);

	clearECC_Point_Hom(M_Point);

	return M_Prime;
}

}


// All the infrastructre is now complete, now we use what we have to perform OT.


// Request a set of params from the Receiver who will run setupDec.
struct decParams_ECC *senderCRS_ECC_Syn_Dec(int writeSocket, int readSocket, int securityParam, gmp_randstate_t state)
{	
	struct decParams_ECC *params = setupDec_ECC( securityParam, state );


	sendDecParams_ECC(writeSocket, readSocket, params);


	return params;
}


struct messyParams_ECC *senderCRS_ECC_Syn_Messy(int writeSocket, int readSocket)
{
	struct messyParams_ECC *params = receiveMessyParams_ECC(writeSocket, readSocket);

	return params;
}


void senderOT_UC_ECC(int writeSocket, int readSocket,
				unsigned char *input0Bytes, unsigned char *input1Bytes, int inputLengths,
				struct decParams_ECC *params, gmp_randstate_t *state)
{
	struct otKeyPair_ECC *keyPair = initKeyPair_ECC();
	struct u_v_Pair_ECC_Hom *c_0, *c_1;
	mpz_t *outputMPZ, *tempMPZ = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_t *input0 = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_t *input1 = (mpz_t*) calloc(1, sizeof(mpz_t));
	unsigned char *curBytes;
	int curLength, bufferOffset;


	mpz_init(*tempMPZ);
	mpz_init(*input0);
	mpz_init(*input1);


	bufferOffset = 0;
	curBytes = receiveBoth(readSocket, curLength);
	keyPair -> pk -> g = deserialise_ECC_Point_Hom(curBytes, &bufferOffset);	

	bufferOffset = 0;
	curBytes = receiveBoth(readSocket, curLength);
	keyPair -> pk -> h = deserialise_ECC_Point_Hom(curBytes, &bufferOffset);

	convertBytesToMPZ(input0, input0Bytes, inputLengths);
	convertBytesToMPZ(input1, input1Bytes, inputLengths);

	c_0 = PVW_OT_Enc_ECC(*input0, params -> crs, params -> params, *state, keyPair -> pk, 0x00);
	c_1 = PVW_OT_Enc_ECC(*input1, params -> crs, params -> params, *state, keyPair -> pk, 0x01);


	bufferOffset = 0;
	curLength = sizeOfSerial_ECC_U_V_Pair_Hom(c_0);
	curLength += sizeOfSerial_ECC_U_V_Pair_Hom(c_1);
	curBytes = (unsigned char *) calloc(curLength, sizeof(unsigned char));
	serialise_U_V_pair_ECC_Hom(c_0, curBytes, &bufferOffset);
	serialise_U_V_pair_ECC_Hom(c_1, curBytes, &bufferOffset);
	sendBoth(writeSocket, (octet*) curBytes, curLength);
}


void bulk_senderOT_UC_ECC(unsigned char *input0Bytes, unsigned char *input1Bytes, int inputLengths,
					struct decParams_ECC *params, gmp_randstate_t *state,
					struct otKeyPair_ECC *keyPair,
					struct u_v_Pair_ECC_Hom **c_i_Array, int u_v_index)
{
	mpz_t *outputMPZ, *tempMPZ;
	mpz_t *input0 = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_t *input1 = (mpz_t*) calloc(1, sizeof(mpz_t));
	int curLength;

	mpz_init(*input0);
	mpz_init(*input1);


	convertBytesToMPZ(input0, input0Bytes, inputLengths);
	convertBytesToMPZ(input1, input1Bytes, inputLengths);


	c_i_Array[u_v_index + 0] = PVW_OT_Enc_ECC(*input0, params -> crs, params -> params, *state, keyPair -> pk, 0x00);
	c_i_Array[u_v_index + 1] = PVW_OT_Enc_ECC(*input1, params -> crs, params -> params, *state, keyPair -> pk, 0x01);
}


struct decParams_ECC *receiverCRS_ECC_Syn_Dec(int writeSocket, int readSocket)//, int securityParam, gmp_randstate_t state)
{
	struct decParams_ECC *params = receiveDecParams_ECC(writeSocket, readSocket);

	return params;
}


struct messyParams_ECC *receiverCRS_ECC_Syn_Messy(int writeSocket, int readSocket, int securityParam, gmp_randstate_t state)
{
	struct messyParams_ECC *params = setupMessy_ECC( securityParam, state );

	sendMessyParams_ECC(writeSocket, readSocket, params);

	return params;
}


unsigned char *receiverOT_UC_ECC(int writeSocket, int readSocket,
							unsigned char inputBit, struct decParams_ECC *params,
							int *outputLength, gmp_randstate_t *state)
{
	struct otKeyPair_ECC *keyPair = keyGen_ECC(params -> crs, inputBit, params -> params, *state);
	
	struct u_v_Pair_ECC_Hom *c_0, *c_1;
	mpz_t *outputMPZ;
	unsigned char *outputBytes, *curBytes;
	int curLength, bufferOffset;
	

	bufferOffset = 0;
	curLength = sizeOfSerial_ECCPoint_Hom(keyPair -> pk -> g);
	curBytes = (unsigned char *) calloc(curLength, sizeof(unsigned char));
	serialise_ECC_Point_Hom(keyPair -> pk -> g, curBytes, &bufferOffset);
	sendBoth(writeSocket, (octet*) curBytes, curLength);


	bufferOffset = 0;
	curLength += sizeOfSerial_ECCPoint_Hom(keyPair -> pk -> h);
	curBytes = (unsigned char *) calloc(curLength, sizeof(unsigned char));
	serialise_ECC_Point_Hom(keyPair -> pk -> h, curBytes, &bufferOffset);
	sendBoth(writeSocket, (octet*) curBytes, curLength);


	curBytes = receiveBoth(readSocket, curLength);
	bufferOffset = 0;
	c_0 = deserialise_U_V_pair_ECC_Hom(curBytes, &bufferOffset);
	c_1 = deserialise_U_V_pair_ECC_Hom(curBytes, &bufferOffset);
	

	if(0x00 == inputBit)
		outputMPZ = PVW_OT_Dec_ECC(c_0, params -> crs, params -> params, keyPair -> sk);
	else
		outputMPZ = PVW_OT_Dec_ECC(c_1, params -> crs, params -> params, keyPair -> sk);

	outputBytes = convertMPZToBytes(*outputMPZ, outputLength);

	return outputBytes;
}

// Perform the first half of the receiver side of the OT, serialise the results into a 
// buffer for sending to the Sender.
struct otKeyPair_ECC *bulk_one_receiverOT_UC_ECC(unsigned char inputBit,
								struct decParams_ECC *params, gmp_randstate_t *state)
{
	struct otKeyPair_ECC *keyPair = keyGen_ECC(params -> crs, inputBit, params -> params, *state);

	return keyPair;
}


// Perform the second half of the receiver side of the OT, deserialising the buffer returned by
// the sender and using this to extract the requested data.
unsigned char *bulk_two_receiverOT_UC_ECC(unsigned char *inputBuffer, int *bufferOffset,
							struct otKeyPair_ECC *keyPair, struct decParams_ECC *params,
							unsigned char inputBit, int *outputLength)
{
	struct u_v_Pair_ECC_Hom *c_0, *c_1;
	mpz_t *outputMPZ, *tempMPZ;
	unsigned char *outputBytes, *curBytes;
	int curLength;


	c_0 = deserialise_U_V_pair_ECC_Hom(inputBuffer, bufferOffset);
	c_1 = deserialise_U_V_pair_ECC_Hom(inputBuffer, bufferOffset);


	if(0x00 == inputBit)
		outputMPZ = PVW_OT_Dec_ECC(c_0, params -> crs, params -> params, keyPair -> sk);
	else
		outputMPZ = PVW_OT_Dec_ECC(c_1, params -> crs, params -> params, keyPair -> sk);


	outputBytes = convertMPZToBytes(*outputMPZ, outputLength);

	return outputBytes;
}



// Testing function. No gurantee this still works.
int testOT_PWV_DDH_Local_ECC()
{
	mpz_t *input0 = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_t *input1 = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_t *decSigma = (mpz_t*) calloc(1, sizeof(mpz_t));
	struct u_v_Pair_ECC_Hom *y0, *y1;
	struct otKeyPair_ECC *keyPair;
	
	unsigned char *input0Bytes = generateRandBytes(16, 16);
	unsigned char *input1Bytes = generateRandBytes(16, 16);
	unsigned char *outputBytes1, *outputBytes2;
	int tempInt1, tempInt2;

	gmp_randstate_t *state = seedRandGen();
	struct decParams_ECC *params;

	params = setupDec_ECC( 1024, *state );


	convertBytesToMPZ(input0, input0Bytes, 16);
	convertBytesToMPZ(input1, input1Bytes, 16);

	keyPair = keyGen_ECC(params -> crs, 0x00, params -> params, *state);

	y0 = PVW_OT_Enc_ECC(*input0, params -> crs, params -> params, *state, keyPair -> pk, 0x00);
	y1 = PVW_OT_Enc_ECC(*input1, params -> crs, params -> params, *state, keyPair -> pk, 0x01);

	decSigma = PVW_OT_Dec_ECC(y0, params -> crs, params -> params, keyPair -> sk);

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

// Testing function for single OT. No gurantee this still works.
void testSender_OT_PVW_ECC()
{
	struct sockaddr_in destWrite, destRead;
	int writeSocket, readSocket, mainWriteSock, mainReadSock;
	int writePort, readPort, i;
	gmp_randstate_t *state = seedRandGen();

	writePort = 7654;
	readPort = writePort + 1;

	unsigned char *input0Bytes = generateRandBytes(16, 16);
	unsigned char *input1Bytes = generateRandBytes(16, 16);
	struct decParams_ECC *params;

	set_up_server_socket(destWrite, writeSocket, mainWriteSock, writePort);
	set_up_server_socket(destRead, readSocket, mainReadSock, readPort);

	params = senderCRS_ECC_Syn_Dec(writeSocket, readSocket, 1024, *state);

	senderOT_UC_ECC(writeSocket, readSocket, input0Bytes, input1Bytes, 16, params, state);

	for(i = 0; i < 16; i ++)
	{
		printf("%02X", input0Bytes[i]);
	}
	printf("\n");

	for(i = 0; i < 16; i ++)
	{
		printf("%02X", input1Bytes[i]);
	}
	printf("\n");


	close_server_socket(writeSocket, mainWriteSock);
	close_server_socket(readSocket, mainReadSock);
}



// Testing function for single OT. No gurantee this still works.
void testReceive_OT_PVW_ECC(char *ipAddress)
{
	int writeSocket, readSocket, writePort, readPort;
	struct sockaddr_in serv_addr_write;
	struct sockaddr_in serv_addr_read;

	readPort = 7654;
	writePort = readPort + 1;
	unsigned char inputBit = 0x01;
	unsigned char *output;
	struct decParams_ECC *params;
	int outputLength;
	int i;

	gmp_randstate_t *state = seedRandGen();

    set_up_client_socket(readSocket, ipAddress, readPort, serv_addr_read);
    set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr_write);

	params = receiverCRS_ECC_Syn_Dec(writeSocket, readSocket);//, 1024, *state);

	output = receiverOT_UC_ECC(writeSocket, readSocket, inputBit, params, &outputLength, state);

	for(i = 0; i < 16; i ++)
	{
		printf("%02X", output[i]);
	}
	printf("\n");


	close_client_socket(readSocket);
	close_client_socket(writeSocket);
}
