unsigned char *generate_Mod_J_Set(int stat_SecParam)
{
	unsigned char *J_set = (unsigned char *) calloc(stat_SecParam, sizeof(int));
	int i = 0, J_set_size;
	unsigned int tempInt = 0;


	do
	{
		J_set_size = 0;
		for(i = 0; i < stat_SecParam; i ++)
		{
			J_set[i] = rand() % 2;
			if(1 == J_set[i])
			{
				J_set_size ++;
			}
		}
	} while(0 == J_set_size || stat_SecParam == J_set_size);

	return J_set;
}


struct params_CnC_ECC *setup_CnC_OT_Mod_Receiver(int stat_SecParam,	int comp_SecParam, gmp_randstate_t state)
{
	struct params_CnC_ECC *params = initParams_CnC_ECC(stat_SecParam, comp_SecParam, state);
	int i = 0, groupBytesLen = 0, CRS_BytesLen = 0, tempInt = 0;
	mpz_t alpha;

	mpz_init(alpha);


	params -> crs -> J_set = generate_Mod_J_Set(stat_SecParam);

	do
	{
		mpz_urandomm(params -> y, state, params -> params -> n);
	} while( 0 == mpz_cmp_ui( params -> y, 0) );


	params -> crs -> g_1 = fixedPointMultiplication(gPreComputes, params -> y, params -> params);
	// params -> crs -> g_1 = windowedScalarPoint(params -> y, params -> params -> g, params -> params);

	for(i = 0; i < stat_SecParam; i ++)
	{	
		do
		{
			mpz_urandomm(alpha, state, params -> params -> n);
		} while( 0 == mpz_cmp_ui( alpha, 0) );


		params -> crs -> h_0_List[i] = fixedPointMultiplication(gPreComputes, alpha, params -> params);
		// params -> crs -> h_0_List[i] = windowedScalarPoint(alpha, params -> params -> g, params -> params);

		// Note that everyone gets a witness...this might not be good.
		// Insert Legal aid for all joke here.
		mpz_set(params -> crs -> alphas_List[i], alpha);

		if( 0x00 == params -> crs -> J_set[i] )
		{
			mpz_add_ui(alpha, alpha, 1);
		}

		params -> crs -> h_1_List[i] = windowedScalarPoint(alpha, params -> crs -> g_1, params -> params);
	}


	return params;
}


struct params_CnC_ECC *setup_CnC_OT_Mod_Full_Receiver(int writeSocket, int readSocket,
													int stat_SecParam,	int comp_SecParam,
													gmp_randstate_t state)
{
	struct params_CnC_ECC *params_R = setup_CnC_OT_Mod_Receiver(stat_SecParam, comp_SecParam, state);
	unsigned char *commBuffer;
	int bufferLength = 0;

	commBuffer = serialiseParams_CnC_ECC(params_R, &bufferLength);
	sendBoth(writeSocket, commBuffer, bufferLength);
	free(commBuffer);


	ZKPoK_DL_Prover(writeSocket, readSocket,
					params_R -> params -> g, params_R -> crs -> g_1,
					params_R -> y, params_R -> params, state);


	return params_R;
}



struct tildeCRS *setup_tildeCRS_Receiver(struct params_CnC_ECC *params_R, int numInputs,
										struct idAndValue *startOfInputChain,
										gmp_randstate_t state)
{
	struct tildeCRS *alteredCRS = initTildeCRS(numInputs, params_R -> params, state);
	unsigned char inputBit;
	int i = 0;
	struct idAndValue *curInput = startOfInputChain -> next;


	for(i = 0; i < numInputs; i ++)
	{
		inputBit = curInput -> value;
		alteredCRS -> lists[i] = initTildeList(params_R -> crs -> stat_SecParam,  alteredCRS -> r_List[i],
											params_R -> crs, params_R -> params, inputBit);
		curInput = curInput -> next;
	}

	return alteredCRS;
}



struct jSetCheckTildes *transfer_CheckValues_CnC_OT_Mod_Receiver(struct params_CnC_ECC *params_R, gmp_randstate_t state)
{
	struct jSetCheckTildes *checkTildes = (struct jSetCheckTildes *) calloc(1, sizeof(struct jSetCheckTildes));
	struct eccPoint *invG1, *hMulInvG1;
	int j, jDoubled = 0;


	checkTildes -> roe_jList = (mpz_t*) calloc(params_R -> crs -> stat_SecParam, sizeof(mpz_t));
	checkTildes -> h_tildeList = (struct eccPoint **) calloc(2 * params_R -> crs -> stat_SecParam, sizeof(struct eccPoint *));

	invG1 = invertPoint(params_R -> crs -> g_1, params_R -> params);

	for(j = 0; j < params_R -> crs -> stat_SecParam; j ++)
	{
		mpz_init(checkTildes -> roe_jList[j]);
		mpz_urandomm(checkTildes -> roe_jList[j], state, params_R -> params -> n);

		checkTildes -> h_tildeList[jDoubled] = windowedScalarPoint(checkTildes -> roe_jList[j], params_R -> crs -> h_0_List[j], params_R -> params);
		
		if(0x00 == params_R -> crs -> J_set[j])
		{
			hMulInvG1 = groupOp(params_R -> crs -> h_1_List[j], invG1, params_R -> params);
			checkTildes -> h_tildeList[jDoubled + 1] = windowedScalarPoint(checkTildes -> roe_jList[j], hMulInvG1, params_R -> params);
			clearECC_Point(hMulInvG1);
		}
		else
		{
			checkTildes -> h_tildeList[jDoubled + 1] = windowedScalarPoint(checkTildes -> roe_jList[j], params_R -> crs -> h_1_List[j], params_R -> params);
		}

		jDoubled += 2;
	}


	clearECC_Point(invG1);

	return checkTildes;
}




unsigned char *decU_W_Pair(struct eccPoint *u, unsigned char *w, mpz_t powToDec,
						const int msgLength, struct eccParams *params)
{

	struct eccPoint *tempPoint;
	unsigned char *rawBytes, *hashedBytes, *output;
	int tempLength = 0;


	tempPoint = windowedScalarPoint(powToDec, u, params);
	rawBytes = convertMPZToBytes(tempPoint -> x, &tempLength);
	hashedBytes = sha256_full(rawBytes, tempLength);
	output = XOR_TwoStrings(hashedBytes, w, msgLength);


	clearECC_Point(tempPoint);
	free(rawBytes);
	free(hashedBytes);

	return output;
}




unsigned char *output_CnC_OT_Mod_Dec_i_j(struct params_CnC_ECC *params_R, mpz_t r_i,
										struct CnC_OT_Mod_CTs *CTs, const int msgLength,
										unsigned char inputBit,
										int j)
{
	if(0x00 == inputBit)
	{
		return decU_W_Pair(CTs -> u_0, CTs -> w_0, r_i, msgLength, params_R -> params);
	}
	else
	{
		return decU_W_Pair(CTs -> u_1, CTs -> w_1, r_i, msgLength, params_R -> params);
	}
}



unsigned char *output_CnC_OT_Mod_Dec_i_j_Alt(struct params_CnC_ECC *params_R, mpz_t r_i,
											struct CnC_OT_Mod_CTs *CTs, const int msgLength,
											unsigned char inputBit,
											int j)
{
	unsigned char *output = NULL;
	mpz_t r_iDotZ, y_inv, unmodded;


	mpz_init(unmodded);
	mpz_init(r_iDotZ);
	mpz_init(y_inv);

	if(0x00 == inputBit)
	{
		if(0x01 == params_R -> crs -> J_set[j])
		{
			mpz_invert(y_inv, params_R -> y, params_R -> params -> n);
			mpz_mul(unmodded, r_i, y_inv);
			mpz_mod(r_iDotZ, unmodded, params_R -> params -> n);

			output = decU_W_Pair(CTs -> u_1, CTs -> w_1, r_iDotZ, msgLength, params_R -> params);
		}
	}
	else
	{
		if(0x01 == params_R -> crs -> J_set[j])
		{
			mpz_mul(unmodded, r_i, params_R -> y);
			mpz_mod(r_iDotZ, unmodded, params_R -> params -> n);

			output = decU_W_Pair(CTs -> u_0, CTs -> w_0, r_iDotZ, msgLength, params_R -> params);
		}
	}


	mpz_clear(y_inv);
	mpz_clear(unmodded);
	mpz_clear(r_iDotZ);

	return output;
}


unsigned char **decrypt_jSet_Checks(struct params_CnC_ECC *params_R, struct CnC_OT_Mod_Check_CT **checkCTs,
									struct jSetCheckTildes *checkTildes)
{
	unsigned char **output = (unsigned char **) calloc(params_R -> crs -> stat_SecParam, sizeof(unsigned char *));
	int i, j;

	for(i = 0; i < params_R -> crs -> stat_SecParam; i ++)
	{
		if(0x00 == params_R -> crs -> J_set[i])
		{
			output[i] = decU_W_Pair(checkCTs[i] -> u, checkCTs[i] -> w, checkTildes -> roe_jList[i], 16, params_R -> params);
		}
		else
		{
			output[i] = NULL;
		}
	}


	return output;
}




void test_CnC_OT_Mod_Receiver(char *ipAddress)
{
	struct params_CnC_ECC *params_R;
	struct tildeList *testTildeList;
	struct jSetCheckTildes *checkTildes;
	struct CnC_OT_Mod_CTs **CTs;
	struct CnC_OT_Mod_Check_CT **checkCTs;
	mpz_t r_i;
	unsigned char **output0, **output1;

	int writeSocket, readSocket, writePort, readPort;
	struct sockaddr_in serv_addr_write;
	struct sockaddr_in serv_addr_read;


	unsigned char *commBuffer, inputBit = 0x01;
	int bufferOffset = 0, commBufferLen = 0;
	int stat_SecParam = 4, comp_SecParam = 256;
	int i, j = 0, k = 0;

	const int DEBUG_TRANSFER = 1;
	const int DEBUG_CHECK = 0;

	readPort = 7654;
	writePort = readPort + 1;
	gmp_randstate_t *state = seedRandGen();


	set_up_client_socket(readSocket, ipAddress, readPort, serv_addr_read);
	set_up_client_socket(writeSocket, ipAddress, writePort, serv_addr_write);


	params_R = setup_CnC_OT_Mod_Full_Receiver(writeSocket, readSocket, stat_SecParam, comp_SecParam, *state);

	mpz_init(r_i);


	output0 = (unsigned char **) calloc(stat_SecParam, sizeof(unsigned char *));
	output1 = (unsigned char **) calloc(stat_SecParam, sizeof(unsigned char *));

	for(i = 0; i < 32; i ++)
	{
		mpz_urandomm(r_i, *state, params_R -> params -> n);

		testTildeList = initTildeList(stat_SecParam, r_i, params_R -> crs, params_R -> params, inputBit);

		commBufferLen = 0;
		commBuffer = serialiseTildeList(testTildeList, stat_SecParam, &commBufferLen);
		sendBoth(writeSocket, commBuffer, commBufferLen);
		free(commBuffer);

		// ZKPoK Here
		ZKPoK_Ext_DH_TupleProver_2U(writeSocket, readSocket, stat_SecParam, &r_i, inputBit,
									params_R -> params -> g, params_R -> crs -> g_1,
									testTildeList -> g_tilde, testTildeList -> g_tilde,
									params_R -> crs -> h_0_List, params_R -> crs -> h_1_List,
									testTildeList -> h_tildeList, params_R -> params, state);
		bufferOffset = 0;
		commBuffer = receiveBoth(readSocket, commBufferLen);
		CTs = deserialise_Mod_CTs(commBuffer, &bufferOffset, 16);
		free(commBuffer);


		#pragma omp parallel for private(j) schedule(auto)
		for(j = 0; j < stat_SecParam; j ++)
		{
			if(0x00 == inputBit)
			{
				output0[j] = output_CnC_OT_Mod_Dec_i_j(params_R, r_i, CTs[j], 16, inputBit, j);
				output1[j] = output_CnC_OT_Mod_Dec_i_j_Alt(params_R, r_i, CTs[j], 16, inputBit, j);
			}
			else
			{
				output0[j] = output_CnC_OT_Mod_Dec_i_j_Alt(params_R, r_i, CTs[j], 16, inputBit, j);
				output1[j] = output_CnC_OT_Mod_Dec_i_j(params_R, r_i, CTs[j], 16, inputBit, j);
			}
		}
	}


	for(j = 0; j < stat_SecParam; j ++)
	{
		if(DEBUG_TRANSFER && NULL != output0[j])
		{
			printf("M_%d_0 = ", j);
			for(i = 0; i < 16; i ++)
			{
				printf("%02X", output0[j][i]);
			}printf("\n");
		}
		if(DEBUG_TRANSFER && NULL != output1[j])
		{
			printf("M_%d_1 = ", j);
			for(i = 0; i < 16; i ++)
			{
				printf("%02X", output1[j][i]);
			}printf("\n\n");
		}
	}


	checkTildes = transfer_CheckValues_CnC_OT_Mod_Receiver(params_R, *state);
	commBuffer = serialise_jSet_CheckTildes(checkTildes, params_R -> crs -> stat_SecParam, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);

	bufferOffset = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	checkCTs = deserialise_OT_Mod_Check_CTs(commBuffer, &bufferOffset, 16);
	free(commBuffer);

	output0 = decrypt_jSet_Checks(params_R, checkCTs, checkTildes);
	for(j = 0; j < stat_SecParam; j ++)
	{
		if(DEBUG_CHECK && NULL != output0[j])
		{
			printf("X_%d   = ", j);
			for(i = 0; i < 16; i ++)
			{
				printf("%02X", output0[j][i]);
			}printf("\n");
		}
	}

	ZKPoK_Ext_DH_TupleProverAll(writeSocket, readSocket, stat_SecParam, checkTildes -> roe_jList, params_R -> crs -> alphas_List,
								params_R -> params -> g, params_R -> crs -> g_1,
								checkTildes -> h_tildeList,
								params_R -> params, state);


	close_client_socket(readSocket);
	close_client_socket(writeSocket);

	freeParams_CnC_ECC(params_R);
	gmp_randclear(*state);
	free(state);
}


