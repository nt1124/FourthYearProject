struct otKeyPair_ECC *keyGen_CnC_OT_ECC(struct params_CnC_ECC *params, unsigned char sigmaBit, gmp_randstate_t state, int j)
{
	struct otKeyPair_ECC *keyPair = initKeyPair_ECC();

	do
	{
		mpz_urandomm(keyPair -> sk, state, params -> params -> p);
	} while( 0 == mpz_cmp_ui(keyPair -> sk, 0) );

	
	if(0x00 == sigmaBit)
	{
		keyPair -> pk -> g = fixedPointMultiplication(gPreComputes, keyPair -> sk, params -> params);
		keyPair -> pk -> h = windowedScalarPoint(keyPair -> sk, params -> crs -> h_0_List[j], params -> params);
	}
	else if(0x01 == sigmaBit)
	{
		keyPair -> pk -> g = windowedScalarPoint(keyPair -> sk, params -> crs -> g_1, params -> params);
		keyPair -> pk -> h = windowedScalarPoint(keyPair -> sk, params -> crs -> h_1_List[j], params -> params);
	}

	return keyPair;
}


struct ECC_PK *setPrimitivePK_ECC_CnC_OT(struct params_CnC_ECC *params, struct PVM_OT_PK_ECC *otPK, unsigned char sigmaBit, int j)
{
	struct ECC_PK *pk = (struct ECC_PK*) calloc(1, sizeof(struct ECC_PK));


	pk -> g_x = copyECC_Point(otPK -> g);
	pk -> h_x = copyECC_Point(otPK -> h);

	if(0 == sigmaBit)
	{
		pk -> g = copyECC_Point(params -> params -> g);
		pk -> h = copyECC_Point(params -> crs -> h_0_List[j]);
	}
	else
	{
		pk -> g = copyECC_Point(params -> crs -> g_1);
		pk -> h = copyECC_Point(params -> crs -> h_1_List[j]);
	}


	return pk;
}


// Could we speed up by passing in the DDH_PK Blue Peter-ed?
struct u_v_Pair_ECC *CnC_OT_Enc_ECC(mpz_t M,
									struct params_CnC_ECC *params, gmp_randstate_t state,
									struct PVM_OT_PK_ECC *otPK, unsigned char sigmaBit, int j)
{
	struct u_v_Pair_ECC *CT;
	struct ECC_PK *pk = setPrimitivePK_ECC_CnC_OT(params, otPK, sigmaBit, j);
	struct eccPoint *msgPoint = mapMPZ_To_Point(M, params -> params);

	CT = ECC_Enc(pk, msgPoint, params -> params, state);


	clearECC_Point(msgPoint);
	clearECC_Point(pk -> g);
	clearECC_Point(pk -> g_x);
	clearECC_Point(pk -> h);
	clearECC_Point(pk -> h_x);
	
	return CT;
}


mpz_t *CnC_OT_Dec_ECC(struct u_v_Pair_ECC *CT, struct params_CnC_ECC *params, mpz_t sk)
{
	mpz_t *M_Prime = (mpz_t*) calloc(1, sizeof(mpz_t));	
	struct eccPoint *M_Point = ECC_Dec(sk, CT, params -> params);
	
	mpz_init_set(*M_Prime, M_Point -> x);

	clearECC_Point(M_Point);

	return M_Prime;
}


// mpz_t *ECC_CnC_OT_Dec_Alt(struct u_v_Pair *CT, struct params_CnC *params, PVM_OT_SK *sk, unsigned char sigmaBit)
mpz_t *CnC_OT_Dec_ECC_Alt(struct u_v_Pair_ECC *CT, struct params_CnC_ECC *params, mpz_t sk, unsigned char sigmaBit)
{
	mpz_t *M_Prime = (mpz_t*) calloc(1, sizeof(mpz_t));	
	struct eccPoint *M_Point = ECC_Dec_Alt(sk, params -> y, CT, params -> params, sigmaBit);
	
	mpz_init_set(*M_Prime, M_Point -> x);

	clearECC_Point(M_Point);

	return M_Prime;
}


struct params_CnC_ECC *setup_CnC_OT_Receiver_ECC(int stat_SecParam, unsigned char *J_set, int comp_SecParam, gmp_randstate_t state)
{
	struct params_CnC_ECC *params = initParams_CnC_ECC(stat_SecParam, comp_SecParam, state);
	int i = 0, groupBytesLen = 0, CRS_BytesLen = 0, tempInt = 0;
	mpz_t alpha;

	mpz_init(alpha);


	params -> crs -> J_set = J_set;

	do
	{
		mpz_urandomm(params -> y, state, params -> params -> n);
	} while( 0 == mpz_cmp_ui( params -> y, 0) );

	params -> crs -> g_1 = fixedPointMultiplication(gPreComputes, params -> y, params -> params);

	for(i = 0; i < stat_SecParam; i ++)
	{	
		do
		{
			mpz_urandomm(alpha, state, params -> params -> n);
		} while( 0 == mpz_cmp_ui( alpha, 0) );

		params -> crs -> h_0_List[i] = fixedPointMultiplication(gPreComputes, alpha, params -> params);

		mpz_set(params -> crs -> alphas_List[i], alpha);

		if( 0x00 == params -> crs -> J_set[i] )
		{
			mpz_add_ui(alpha, alpha, 1);
		}

		params -> crs -> h_1_List[i] = windowedScalarPoint(alpha, params -> crs -> g_1, params -> params);
	}


	return params;
}


// Really this is just deserialising a params_CnC from bufferReceived
struct params_CnC_ECC *setup_CnC_OT_Sender_ECC(unsigned char *bufferReceived)
{
	struct params_CnC_ECC *params = (struct params_CnC_ECC*) calloc(1, sizeof(struct params_CnC_ECC));
	int i, bufferOffset = 0;

	params -> params = deserialiseECC_Params(bufferReceived, &bufferOffset);
	params -> crs = deserialise_CRS_ECC(bufferReceived, &bufferOffset);

	return params;
}


// Perform the first half of the receiver side of the OT, serialise the results into a 
// buffer for sending to the Sender.
struct otKeyPair_ECC *CnC_OT_Transfer_One_Receiver_ECC(unsigned char inputBit, int j,
								struct params_CnC_ECC *params, gmp_randstate_t *state)
{
	struct otKeyPair_ECC *keyPair = keyGen_CnC_OT_ECC(params, inputBit, *state, j);

	return keyPair;
}


void CnC_OT_Transfer_One_Sender_ECC(unsigned char *input0Bytes, unsigned char *input1Bytes, int inputLengths,
									struct params_CnC_ECC *params, gmp_randstate_t *state,
									struct otKeyPair_ECC *keyPair,
									struct u_v_Pair_ECC **c_i_Array, int u_v_index,
									int j)
{
	mpz_t *outputMPZ, *tempMPZ;
	mpz_t *input0 = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_t *input1 = (mpz_t*) calloc(1, sizeof(mpz_t));

	mpz_init(*input0);
	mpz_init(*input1);

	convertBytesToMPZ(input0, input0Bytes, inputLengths);
	convertBytesToMPZ(input1, input1Bytes, inputLengths);

	c_i_Array[u_v_index + 0] = CnC_OT_Enc_ECC(*input0, params, *state, keyPair -> pk, 0x00, j);
	c_i_Array[u_v_index + 1] = CnC_OT_Enc_ECC(*input1, params, *state, keyPair -> pk, 0x01, j);
}


// Perform the second half of the receiver side of the OT, deserialising the buffer returned by
// the sender and using this to extract the requested data.
unsigned char *CnC_OT_Output_One_Receiver_0_ECC(struct u_v_Pair_ECC *c_0, unsigned char inputBit,
								struct otKeyPair_ECC *keyPair, struct params_CnC_ECC *params, int j)
{
	mpz_t *outputMPZ;
	unsigned char *outputBytes = NULL;
	int curLength, k;


	if(0x00 == inputBit)
	{
		outputMPZ = CnC_OT_Dec_ECC(c_0, params, keyPair -> sk);
		outputBytes = convertMPZToBytes(*outputMPZ, &curLength);
	}
	else if(0x01 == params -> crs -> J_set[j])
	{
		outputMPZ = CnC_OT_Dec_ECC_Alt(c_0, params, keyPair -> sk, 0x01);
		outputBytes = convertMPZToBytes(*outputMPZ, &curLength);
	}

	return outputBytes;
}


// Perform the second half of the receiver side of the OT, deserialising the buffer returned by
// the sender and using this to extract the requested data.
unsigned char *CnC_OT_Output_One_Receiver_1_ECC(struct u_v_Pair_ECC *c_1, unsigned char inputBit,
								struct otKeyPair_ECC *keyPair, struct params_CnC_ECC *params, int j)
{
	mpz_t *outputMPZ;
	unsigned char *outputBytes = NULL;
	int curLength, k;


	if(0x01 == inputBit)
	{
		outputMPZ = CnC_OT_Dec_ECC(c_1, params, keyPair -> sk);
		outputBytes = convertMPZToBytes(*outputMPZ, &curLength);
	}
	else if(0x01 == params -> crs -> J_set[j])
	{
		outputMPZ = CnC_OT_Dec_ECC_Alt(c_1, params, keyPair -> sk, 0x00);
		outputBytes = convertMPZToBytes(*outputMPZ, &curLength);
	}


	return outputBytes;
}


// Perform the second half of the receiver side of the OT, deserialising the buffer returned by
// the sender and using this to extract the requested data.
void CnC_OT_Output_One_Receiver_ECC(struct u_v_Pair_ECC *c_0, struct u_v_Pair_ECC *c_1,
								struct otKeyPair_ECC *keyPair, struct params_CnC_ECC *params,
								unsigned char inputBit, int j,
								unsigned char **output_0, unsigned char **output_1)
{
	mpz_t *outputMPZ, *output_j_MPZ;
	unsigned char *outputBytes;
	int curLength, k;


	if(0x00 == inputBit)
	{
		outputMPZ = CnC_OT_Dec_ECC(c_0, params, keyPair -> sk);
		*output_0 = convertMPZToBytes(*outputMPZ, &curLength);

		if(0x01 == params -> crs -> J_set[j])
		{
			output_j_MPZ = CnC_OT_Dec_ECC_Alt(c_1, params, keyPair -> sk, 0x00);
			*output_1 = convertMPZToBytes(*output_j_MPZ, &curLength);
		}
		else
		{
			*output_1 = NULL;
		}
	}
	else
	{
		outputMPZ = CnC_OT_Dec_ECC(c_1, params, keyPair -> sk);
		*output_1 = convertMPZToBytes(*outputMPZ, &curLength);

		if(0x01 == params -> crs -> J_set[j])
		{
			output_j_MPZ = CnC_OT_Dec_ECC_Alt(c_0, params, keyPair -> sk, 0x01);
			*output_0 = convertMPZToBytes(*output_j_MPZ, &curLength);
		}
		else
		{
			*output_0 = NULL;
		}
	}
}


void test_local_CnC_OT_ECC()
{
	struct params_CnC_ECC *params_R, *params_S;
	int i, j, k, numInputs = 8, numTests = 8, comp_SecParam = 1024;
	int totalOTs = numInputs * numTests;

	struct otKeyPair_ECC **keyPairs_R = (struct otKeyPair_ECC **) calloc(numTests, sizeof(struct otKeyPair_ECC*));
	struct otKeyPair_ECC **keyPairs_S;
	struct u_v_Pair_ECC **c_i_Array_S = (struct u_v_Pair_ECC **) calloc(2*numTests, sizeof(struct u_v_Pair_ECC*));
	struct u_v_Pair_ECC **c_i_Array_R;

	unsigned char *inputBytes[numTests][2];
	unsigned char *outputBytes[numTests][2];
	unsigned char *tempChars_0, *tempChars_1, *commBuffer;
	unsigned char sigmaBit = 0x01, *J_set;

	int bufferOffset = 0, u_v_index = 0, tempInt = 0;

	gmp_randstate_t *state = seedRandGen();

	for(i = 0; i < numTests; i ++)
	{
		inputBytes[i][0] = generateRandBytes(16, 16);
		inputBytes[i][1] = generateRandBytes(16, 16);
	}


	J_set = generateJ_Set(stat_SecParam);
	params_R = setup_CnC_OT_Receiver_ECC(numTests, J_set, comp_SecParam, *state);
	commBuffer = serialiseParams_CnC_ECC(params_R, &bufferOffset);


	params_S = setup_CnC_OT_Sender_ECC(commBuffer);
	free(commBuffer);

	for(i = 0; i < numTests; i ++)
	{
		keyPairs_R[i] = CnC_OT_Transfer_One_Receiver_ECC(sigmaBit, i, params_R, state);
	}

	bufferOffset = 0;
	commBuffer = serialise_PKs_otKeyPair_ECC_Array(keyPairs_R, numTests, &bufferOffset);
	keyPairs_S = deserialise_PKs_otKeyPair_ECC_Array(commBuffer, numTests);
	free(commBuffer);


	for(i = 0; i < numTests; i ++)
	{
		CnC_OT_Transfer_One_Sender_ECC(inputBytes[i][0], inputBytes[i][1], 16, params_S, state,
									keyPairs_S[i], c_i_Array_S, u_v_index, i);
		u_v_index += 2;
	}

	bufferOffset = 0;
	u_v_index = 0;
	commBuffer = serialise_U_V_Pair_Array_ECC(c_i_Array_S, numTests * 2, &bufferOffset);
	c_i_Array_R = deserialise_U_V_Pair_Array_ECC(commBuffer, numTests * 2);

	bufferOffset = 0;
	for(i = 0; i < numTests; i ++)
	{
		CnC_OT_Output_One_Receiver_ECC(c_i_Array_R[u_v_index], c_i_Array_R[u_v_index + 1], keyPairs_R[i], params_R, sigmaBit, i, &tempChars_0, &tempChars_1);

		outputBytes[i][0] = tempChars_0;
		outputBytes[i][1] = tempChars_1;
		u_v_index += 2;
	}

	// printTrueTestInputs(numTests, inputBytes, sigmaBit);
	// printTrueTestInputs(numTests, outputBytes, sigmaBit);
	testVictory(numTests, inputBytes, outputBytes, sigmaBit, params_R -> crs -> J_set);
}
