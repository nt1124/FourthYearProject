// Really this is just deserialising a params_CnC from bufferReceived
struct params_CnC_ECC *setup_CnC_OT_Mod_Sender(unsigned char *bufferReceived)
{
	struct params_CnC_ECC *params = (struct params_CnC_ECC*) calloc(1, sizeof(struct params_CnC_ECC));
	int i, bufferOffset = 0;

	params -> params = deserialiseECC_Params(bufferReceived, &bufferOffset);
	params -> crs = deserialise_CRS_ECC(bufferReceived, &bufferOffset);

	return params;
}


struct params_CnC_ECC *setup_CnC_OT_Mod_Full_Sender(int writeSocket, int readSocket, gmp_randstate_t state)
{
	struct params_CnC_ECC *params_S;
	unsigned char *commBuffer;
	int bufferLength = 0, verified = 0;

	commBuffer = receiveBoth(readSocket, bufferLength);
	params_S = setup_CnC_OT_Mod_Sender(commBuffer);
	free(commBuffer);


	verified = ZKPoK_DL_Verifier(writeSocket, readSocket, params_S -> params,
					params_S -> params -> g, params_S -> crs -> g_1, state);

	printf("Verified CnC_OT_Mod setup = %d\n", verified);
	fflush(stdout);


	return params_S;
}


struct CnC_OT_Mod_CTs *transfer_CnC_OT_Mod_Enc_i_j(struct params_CnC_ECC *params_S, const int msgLength,
												struct tildeList *tildes,
												unsigned char *M_0, unsigned char *M_1,
												gmp_randstate_t state, int j)
{
	struct CnC_OT_Mod_CTs *CTs;
	struct ECC_PK *PK;
	struct u_v_Pair_ECC *tempCT;
	unsigned char *v_xBytes, *hashedV_x;
	int tempLength;


	CTs = (struct CnC_OT_Mod_CTs *) calloc(1, sizeof(struct CnC_OT_Mod_CTs ));

	// update_CnC_OT_Mod_PK_With_0(PK, params_S, tildes, j);
	PK = generateFullKey(params_S, tildes, j, 0x00);

	tempCT = randomiseDDH_ECC(PK, params_S -> params, state);
	v_xBytes = convertMPZToBytes(tempCT -> v -> x, &tempLength);
	hashedV_x = sha256_full(v_xBytes, tempLength);

	CTs -> u_0 = tempCT -> u;
	CTs -> w_0 = XOR_TwoStrings(hashedV_x, M_0, msgLength);

	clearECC_Point(tempCT -> v);
	free(hashedV_x);
	free(v_xBytes);
	free(tempCT);


	update_CnC_OT_Mod_PK_With_1(PK, params_S, j);
	tempCT = randomiseDDH_ECC(PK, params_S -> params, state);
	v_xBytes = convertMPZToBytes(tempCT -> v -> x, &tempLength);
	hashedV_x = sha256_full(v_xBytes, tempLength);

	CTs -> u_1 = tempCT -> u;
	CTs -> w_1 = XOR_TwoStrings(hashedV_x, M_1, msgLength);

	clearECC_Point(tempCT -> v);
	free(hashedV_x);
	free(v_xBytes);
	free(tempCT);
	freeECC_PK(PK);


	return CTs;
}

struct CnC_OT_Mod_CTs **transfer_CnC_OT_Mod_Enc_i(struct params_CnC_ECC *params_S, struct tildeList *tildes,
												unsigned char **M_0, unsigned char **M_1, gmp_randstate_t state)
{
	struct CnC_OT_Mod_CTs **CTs;
	struct ECC_PK *PK;
	struct u_v_Pair_ECC *tempCT;
	unsigned char *v_xBytes, *hashedV_x;

	const int stat_SecParam = params_S -> crs -> stat_SecParam, msgLength = 16;
	int j, tempLength;


	CTs = (struct CnC_OT_Mod_CTs **) calloc(stat_SecParam, sizeof(struct CnC_OT_Mod_CTs *));
	PK = generate_Base_CnC_OT_Mod_PK(params_S, tildes);


	for(j = 0; j < stat_SecParam; j ++)
	{
		CTs[j] = (struct CnC_OT_Mod_CTs *) calloc(1, sizeof(struct CnC_OT_Mod_CTs ));

		update_CnC_OT_Mod_PK_With_0(PK, params_S, tildes, j);
		tempCT = randomiseDDH_ECC(PK, params_S -> params, state);
		v_xBytes = convertMPZToBytes(tempCT -> v -> x, &tempLength);
		hashedV_x = sha256_full(v_xBytes, tempLength);

		CTs[j] -> u_0 = tempCT -> u;
		CTs[j] -> w_0 = XOR_TwoStrings(hashedV_x, M_0[j], msgLength);

		clearECC_Point(tempCT -> v);
		free(hashedV_x);
		free(v_xBytes);
		free(tempCT);


		update_CnC_OT_Mod_PK_With_1(PK, params_S, j);
		tempCT = randomiseDDH_ECC(PK, params_S -> params, state);
		v_xBytes = convertMPZToBytes(tempCT -> v -> x, &tempLength);
		hashedV_x = sha256_full(v_xBytes, tempLength);

		CTs[j] -> u_1 = tempCT -> u;
		CTs[j] -> w_1 = XOR_TwoStrings(hashedV_x, M_1[j], msgLength);

		clearECC_Point(tempCT -> v);
		free(hashedV_x);
		free(v_xBytes);
		free(tempCT);
	}

	return CTs;
}


struct CnC_OT_Mod_Check_CT **checkCnC_OT_Mod_Enc(struct params_CnC_ECC *params_S, struct jSetCheckTildes *checkTildes,
												unsigned char **X_js, gmp_randstate_t state)
{
	struct CnC_OT_Mod_Check_CT **checkCTs;
	struct ECC_PK *PK;
	struct u_v_Pair_ECC *tempCT;
	unsigned char *v_xBytes, *hashedV_x;
	struct eccPoint *invG1;

	const int stat_SecParam = params_S -> crs -> stat_SecParam, msgLength = 16;
	int j, tempLength;


	checkCTs = (struct CnC_OT_Mod_Check_CT **) calloc(stat_SecParam, sizeof(struct CnC_OT_Mod_Check_CT *));
	invG1 = invertPoint(params_S -> crs -> g_1, params_S -> params);


	for(j = 0; j < stat_SecParam; j ++)
	{
		checkCTs[j] = (struct CnC_OT_Mod_Check_CT *) calloc(1, sizeof(struct CnC_OT_Mod_Check_CT ));
		PK = generateBase_CnC_OT_Mod_CheckPK(params_S, checkTildes, invG1, j);

		tempCT = randomiseDDH_ECC(PK, params_S -> params, state);
		v_xBytes = convertMPZToBytes(tempCT -> v -> x, &tempLength);
		hashedV_x = sha256_full(v_xBytes, tempLength);

		checkCTs[j] -> u = tempCT -> u;
		checkCTs[j] -> w = XOR_TwoStrings(hashedV_x, X_js[j], msgLength);

		clearECC_Point(PK -> g);
		clearECC_Point(PK -> g_x);
		clearECC_Point(PK -> h);
		clearECC_Point(PK -> h_x);
		free(PK);

		free(hashedV_x);
		free(v_xBytes);

		clearECC_Point(tempCT -> v);
		free(tempCT);

	}


	return checkCTs;
}


void test_CnC_OT_Mod_Sender()
{
	struct params_CnC_ECC *params_S;
	struct tildeList *receivedTildeList;
	struct CnC_OT_Mod_CTs **CTs;
	struct CnC_OT_Mod_Check_CT **checkCTs;
	struct jSetCheckTildes *checkTildes;

	struct sockaddr_in destWrite, destRead;
	int writeSocket, readSocket, mainWriteSock, mainReadSock;
	int writePort, readPort, i, j;

	unsigned char *commBuffer;
	unsigned char **inputX_js, **M_0, **M_1;

	int stat_SecParam = 4, comp_SecParam = 256, verified = 0;
	int bufferOffset = 0, commBufferLen = 0;

	const int DEBUG_TRANSFER = 1;
	const int DEBUG_CHECK = 0;


	inputX_js = (unsigned char **) calloc(stat_SecParam, sizeof(unsigned char*));
	M_0 = (unsigned char **) calloc(stat_SecParam, sizeof(unsigned char*));
	M_1 = (unsigned char **) calloc(stat_SecParam, sizeof(unsigned char*));

	writePort = 7654;
	readPort = writePort + 1;
	gmp_randstate_t *state = seedRandGen();


	set_up_server_socket(destWrite, writeSocket, mainWriteSock, writePort);
	set_up_server_socket(destRead, readSocket, mainReadSock, readPort);


	params_S = setup_CnC_OT_Mod_Full_Sender(writeSocket, readSocket, *state);


	for(j = 0; j < stat_SecParam; j ++)
	{
		inputX_js[j] = generateRandBytes(16, 16);
		M_0[j] = generateRandBytes(16, 16);
		M_1[j] = generateRandBytes(16, 16);

		if(DEBUG_TRANSFER)
		{
			printf("M_%d_0 = ", j);
			for(i = 0; i < 16; i ++)
			{
				printf("%02X", M_0[j][i]);
			}
			printf("\nM_%d_1 = ", j);
			
			for(i = 0; i < 16; i ++)
			{
				printf("%02X", M_1[j][i]);
			}
			printf("\n\n");
		}

		if(DEBUG_CHECK)
		{
			printf("X_%d   = ", j);
			for(i = 0; i < 16; i ++)
			{
				printf("%02X", inputX_js[j][i]);
			}
			printf("\n");
		}

	}

	for(i = 0; i < 32; i ++)
	{
		bufferOffset = 0;
		commBuffer = receiveBoth(readSocket, commBufferLen);
		receivedTildeList = deserialiseTildeList(commBuffer, stat_SecParam, &bufferOffset);
		free(commBuffer);

		// ZKPoK
		verified |= ZKPoK_Ext_DH_TupleVerifier_2U(writeSocket, readSocket, stat_SecParam,
												params_S -> params -> g, params_S -> crs -> g_1,
												receivedTildeList -> g_tilde, receivedTildeList -> g_tilde,
												params_S -> crs -> h_0_List, params_S -> crs -> h_1_List,
												receivedTildeList -> h_tildeList, params_S -> params, state);

		commBufferLen = 0;
		CTs = transfer_CnC_OT_Mod_Enc_i(params_S, receivedTildeList, M_0, M_1, *state);
		commBuffer = serialise_Mod_CTs(CTs, stat_SecParam, &commBufferLen, 16);
		sendBoth(writeSocket, commBuffer, commBufferLen);
		free(commBuffer);
	}
	printf("Sender verify ?= %d\n", verified);

	bufferOffset = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	checkTildes = deserialise_jSet_CheckTildes(commBuffer, &bufferOffset);
	free(commBuffer);


	checkCTs = checkCnC_OT_Mod_Enc(params_S, checkTildes, inputX_js, *state);
	commBuffer = serialise_OT_Mod_Check_CTs(checkCTs, stat_SecParam, &commBufferLen, 16);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);


	ZKPoK_Ext_DH_TupleVerifierAll(writeSocket, readSocket, stat_SecParam,
								params_S -> params -> g, params_S -> crs -> g_1,
								checkTildes -> h_tildeList,
								params_S -> params, state);


	close_server_socket(writeSocket, mainWriteSock);
	close_server_socket(readSocket, mainReadSock);

	freeParams_CnC_ECC(params_S);
	gmp_randclear(*state);
	free(state);
}