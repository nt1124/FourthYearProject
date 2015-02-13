struct otKeyPair *keyGen_CnC_OT(struct params_CnC *params, unsigned char sigmaBit, gmp_randstate_t state, int j)
{
	struct otKeyPair *keyPair = initKeyPair();

	do
	{
		mpz_urandomm( *(keyPair -> sk), state, params -> group -> p);
	} while( 0 == mpz_cmp_ui(*(keyPair -> sk), 0) );

	
	if(0 == sigmaBit)
	{
		mpz_powm(keyPair -> pk -> g, params -> group -> g, *(keyPair -> sk), params -> group -> p);
		mpz_powm(keyPair -> pk -> h, params -> crs -> h_0_List[j], *(keyPair -> sk), params -> group -> p);
	}
	else if(1 == sigmaBit)
	{
		mpz_powm(keyPair -> pk -> g, params -> crs -> g_1, *(keyPair -> sk), params -> group -> p);
		mpz_powm(keyPair -> pk -> h, params -> crs -> h_1_List[j], *(keyPair -> sk), params -> group -> p);
	}

	return keyPair;
}


struct DDH_PK *setPrimitivePK_CnC_OT(struct params_CnC *params, struct PVM_OT_PK *otPK, int sigmaBit, int j)
{
	struct DDH_PK *pk = initPublicKey();


	mpz_set(pk -> g_x, otPK -> g);
	mpz_set(pk -> h_x, otPK -> h);


	if(0 == sigmaBit)
	{
		mpz_set(pk -> g, params -> group -> g);
		mpz_set(pk -> h, params -> crs -> h_0_List[j]);
	}
	else
	{
		mpz_set(pk -> g, params -> crs -> g_1);
		mpz_set(pk -> h, params -> crs -> h_1_List[j]);
	}


	return pk;
}


// Could we speed up by passing in the DDH_PK Blue Peter-ed?
struct u_v_Pair *CnC_OT_Enc(mpz_t M,
							struct params_CnC *params, gmp_randstate_t state,
							struct PVM_OT_PK *otPK, unsigned char sigmaBit, int j)
{
	struct u_v_Pair *CT = init_U_V();

	struct DDH_PK *pk = setPrimitivePK_CnC_OT(params, otPK, sigmaBit, j);

	/*
	if(0x01 == sigmaBit)
	{
		gmp_printf("%d  ->  %Zd\n\n", j, M);
	}
	*/

	CT = encDDH(pk, params -> group, M, state);

	return CT;
}


mpz_t *CnC_OT_Dec(struct u_v_Pair *CT, struct params_CnC *params, PVM_OT_SK *sk)
{
	mpz_t *M_Prime = decDDH(sk, params -> group, CT);
	
	return M_Prime;
}


mpz_t *CnC_OT_Dec_Alt(struct u_v_Pair *CT, struct params_CnC *params, PVM_OT_SK *sk, unsigned char sigmaBit)
{

	// mpz_t *M_Prime = decDDH(sk_z, params -> group, CT);
	mpz_t *M_Prime = decDDH_Alt(sk, params -> y, params -> group, CT, sigmaBit);
	
	return M_Prime;
}


struct params_CnC *setup_CnC_OT_Receiver(int stat_SecParam,	int comp_SecParam, gmp_randstate_t state)
{
	struct params_CnC *params = initParams_CnC(stat_SecParam, comp_SecParam, state);
	int i = 0, groupBytesLen = 0, CRS_BytesLen = 0, tempInt = 0;
	mpz_t alpha;

	mpz_init(alpha);

	params -> crs -> J_set = generateJ_Set(stat_SecParam);

	do
	{
		mpz_urandomm(params -> y, state, params -> group -> p);
	} while( 0 == mpz_cmp_ui( params -> y, 0) );
	mpz_powm(params -> crs -> g_1, params -> group -> g, params -> y, params -> group -> p);


	for(i = 0; i < stat_SecParam; i ++)
	{	
		do
		{
			mpz_urandomm(alpha, state, params -> group -> p);
		} while( 0 == mpz_cmp_ui( alpha, 0) );

		mpz_powm(params -> crs -> h_0_List[i], params -> group -> g, alpha, params -> group -> p);

		/*
		if( 0x00 == params -> crs -> J_set[i] )
		{
			mpz_add_ui(alpha, alpha, 1);
		}
		*/

		mpz_powm(params -> crs -> h_1_List[i], params -> crs -> g_1, alpha, params -> group -> p);
	}

	return params;
}


// Really this is just deserialising a params_CnC from bufferReceived
struct params_CnC *setup_CnC_OT_Sender(unsigned char *bufferReceived)
{
	struct params_CnC *params = (struct params_CnC*) calloc(1, sizeof(struct params_CnC));
	int i, bufferOffset = 0;

	params -> group = deserialise_DDH_Group(bufferReceived, &bufferOffset);
	params -> crs = deserialise_CRS(bufferReceived, &bufferOffset);

	return params;
}


// Perform the first half of the receiver side of the OT, serialise the results into a 
// buffer for sending to the Sender.
struct otKeyPair *CnC_OT_Transfer_One_Receiver(unsigned char inputBit, int j,
								struct params_CnC *params, gmp_randstate_t *state)
{
	struct otKeyPair *keyPair = keyGen_CnC_OT(params, inputBit, *state, j);

	return keyPair;
}



// Perform the second half of the receiver side of the OT, deserialising the buffer returned by
// the sender and using this to extract the requested data.
unsigned char *CnC_OT_Output_One_Receiver(unsigned char *inputBuffer, int *bufferOffset,
											struct otKeyPair *keyPair, struct params_CnC *params,
											unsigned char inputBit, int j,
											unsigned char *output_j)
{
	struct u_v_Pair *c_0, *c_1;
	mpz_t *outputMPZ, *output_j_MPZ, *tempMPZ;
	unsigned char *outputBytes, *curBytes;
	int curLength;


	c_0 = deserialise_U_V_pair(inputBuffer, bufferOffset);
	c_1 = deserialise_U_V_pair(inputBuffer, bufferOffset);


	if(0x00 == inputBit)
	{
		outputMPZ = CnC_OT_Dec(c_0, params, keyPair -> sk);

		if(0 && 0x01 == params -> crs -> J_set[j])
			output_j_MPZ = CnC_OT_Dec_Alt(c_1, params, keyPair -> sk, 0x00);
	}
	else
	{
		outputMPZ = CnC_OT_Dec(c_1, params, keyPair -> sk);

		if(0 && 0x01 == params -> crs -> J_set[j])
			output_j_MPZ = CnC_OT_Dec_Alt(c_0, params, keyPair -> sk, 0x01);
	}

	outputBytes = convertMPZToBytes(*outputMPZ, &curLength);

	// output_j = NULL;

	if(0x01 == params -> crs -> J_set[j])
	{
		// gmp_printf("%d  ->  %Zd\n\n", j, *output_j_MPZ);
		// output_j = convertMPZToBytes(*output_j_MPZ, &curLength);
	}

	return outputBytes;
}


void CnC_OT_Transfer_One_Sender(unsigned char *input0Bytes, unsigned char *input1Bytes, int inputLengths,
								struct params_CnC *params, gmp_randstate_t *state,
								struct otKeyPair *keyPair,
								struct u_v_Pair **c_i_Array, int u_v_index,
								int j)
{
	mpz_t *outputMPZ, *tempMPZ;
	mpz_t *input0 = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_t *input1 = (mpz_t*) calloc(1, sizeof(mpz_t));

	mpz_init(*input0);
	mpz_init(*input1);


	convertBytesToMPZ(input0, input0Bytes, inputLengths);
	convertBytesToMPZ(input1, input1Bytes, inputLengths);

	c_i_Array[u_v_index + 0] = CnC_OT_Enc(*input0, params, *state, keyPair -> pk, 0x00, j);
	c_i_Array[u_v_index + 1] = CnC_OT_Enc(*input1, params, *state, keyPair -> pk, 0x01, j);


	// serialise_U_V_pair(c_0, outputBuffer, outputOffset);
	// serialise_U_V_pair(c_1, outputBuffer, outputOffset);
}




void printTrueTestInputs(int numTests, unsigned char *inputBytes[][2])
{
	int i, j;

	for(i = 0; i < numTests; i ++)
	{
		if(NULL != inputBytes[i][0])
		{
			printf("(%d, 0) >> ", i);
			for(j = 0; j < 16; j ++)
			{
				printf("%02X", inputBytes[i][0][j]);
			}
			printf("\n");
		}

		if(NULL != inputBytes[i][1])
		{
			printf("(%d, 1) >> ", i);
			for(j = 0; j < 16; j ++)
			{
				printf("%02X", inputBytes[i][1][j]);
			}
			printf("\n");
		}
	}
}


// Testing function for single OT. No gurantee this still works.






void test_local_CnC_OT()
{
	struct params_CnC *params_R, *params_S;
	int i, j, numTests = 6, comp_SecParam = 1024;

	struct otKeyPair **keyPairs_R = (struct otKeyPair **) calloc(numTests, sizeof(struct otKeyPair*));
	struct otKeyPair **keyPairs_S;
	struct u_v_Pair **c_i_Array_S = (struct u_v_Pair **) calloc(2*numTests, sizeof(struct u_v_Pair*));

	unsigned char *inputBytes[numTests][2];
	unsigned char *outputBytes[numTests][2];
	unsigned char *tempChars;
	unsigned char *commBuffer;
	unsigned char sigmaBit = 0x01;

	int bufferOffset = 0, u_v_index = 0, tempInt = 0;

	gmp_randstate_t *state = seedRandGen();

	for(i = 0; i < numTests; i ++)
	{
		inputBytes[i][0] = generateRandBytes(16, 16);
		inputBytes[i][1] = generateRandBytes(16, 16);
	}


	params_R = setup_CnC_OT_Receiver(numTests, comp_SecParam, *state);
	commBuffer = serialiseParams_CnC(params_R, &bufferOffset);

	params_S = setup_CnC_OT_Sender(commBuffer);
	free(commBuffer);

	for(i = 0; i < numTests; i ++)
	{
		keyPairs_R[i] = CnC_OT_Transfer_One_Receiver(sigmaBit, i, params_R, state);
	}

	bufferOffset = 0;
	commBuffer = serialise_PKs_otKeyPair_Array(keyPairs_R, numTests, &bufferOffset);
	keyPairs_S = deserialise_PKs_otKeyPair_Array(commBuffer, numTests);
	free(commBuffer);


	for(i = 0; i < numTests; i ++)
	{

		CnC_OT_Transfer_One_Sender(inputBytes[i][0], inputBytes[i][1], 16, params_S, state,
									keyPairs_S[i], c_i_Array_S, u_v_index, i);
		u_v_index += 2;
	}

	bufferOffset = 0;
	commBuffer = serialise_U_V_Pair_Array(c_i_Array_S, numTests * 2, &bufferOffset);

	/*
	mpz_t temp1, temp2, temp3;
	mpz_init(temp1);
	mpz_init(temp2);
	mpz_init(temp3);
	*/

	bufferOffset = 0;
	for(i = 0; i < numTests; i ++)
	{
		outputBytes[i][0] = CnC_OT_Output_One_Receiver(commBuffer, &bufferOffset, keyPairs_R[i], params_R, sigmaBit, i, tempChars);

		if(0x01 == params_R -> crs -> J_set[i])
		{

			// outputBytes[i][0] = CnC_OT_Output_One_Receiver(commBuffer, &bufferOffset, keyPairs_R[i], params_R, sigmaBit, i, tempChars);
			// outputBytes[i][1] = tempChars;
		}
	}

	printTrueTestInputs(numTests, inputBytes);
	for(i = 0; i < numTests; i ++)
	{
		printf("(%d, 0) >> ", i);
		for(j = 0; j < 16; j ++)
		{
			printf("%02X", inputBytes[i][0][j]);
		}
		printf("\n");
	}
	// printTrueTestInputs(numTests, outputBytes);
}



		/*
		mpz_powm(temp1, params_R -> crs -> h_0_List[i], params_R -> y, params_R -> group -> p);
		mpz_invert(temp2, temp1, params_R -> group -> p);
		mpz_mul(temp1, temp2, params_R -> crs -> h_1_List[i]);
		mpz_mod(temp2, temp1, params_R -> group -> p);
		mpz_invert(temp3, params_R -> y, params_R -> group -> p);
		mpz_powm(temp2, params_R -> crs -> h_0_List[i], temp3, params_R -> group -> p);

		mpz_mul(temp1, temp2, params_R -> crs -> h_1_List[i]);
		mpz_mod(temp2, temp1, params_R -> group -> p);

		gmp_printf("%Zd\n\n%Zd\n\n", temp2, params_R -> crs -> h_0_List[i]);
		*/