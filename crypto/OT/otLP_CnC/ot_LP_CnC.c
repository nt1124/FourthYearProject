struct otKeyPair *keyGen_CnC_OT(struct params_CnC *params, unsigned char sigmaBit, gmp_randstate_t state, int j)
{
	struct otKeyPair *keyPair = initKeyPair();

	do
	{
		mpz_urandomm( *(keyPair -> sk), state, params -> group -> p);
	} while( 0 == mpz_cmp_ui(*(keyPair -> sk), 0) );

	
	if(0x00 == sigmaBit)
	{
		mpz_powm(keyPair -> pk -> g, params -> group -> g, *(keyPair -> sk), params -> group -> p);
		mpz_powm(keyPair -> pk -> h, params -> crs -> h_0_List[j], *(keyPair -> sk), params -> group -> p);
	}
	else if(0x01 == sigmaBit)
	{
		mpz_powm(keyPair -> pk -> g, params -> crs -> g_1, *(keyPair -> sk), params -> group -> p);
		mpz_powm(keyPair -> pk -> h, params -> crs -> h_1_List[j], *(keyPair -> sk), params -> group -> p);
	}

	return keyPair;
}


struct DDH_PK *setPrimitivePK_CnC_OT(struct params_CnC *params, struct PVM_OT_PK *otPK, unsigned char sigmaBit, int j)
{
	struct DDH_PK *pk = initPublicKey();


	mpz_set(pk -> g_x, otPK -> g);
	mpz_set(pk -> h_x, otPK -> h);


	if(0x00 == sigmaBit)
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

		mpz_set(params -> crs -> alphas_List[i], alpha);

		if( 0x00 == params -> crs -> J_set[i] )
		{
			mpz_add_ui(alpha, alpha, 1);
		}

		// mpz_set(params -> crs -> alphas_List[i], alpha);

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
}



// Perform the second half of the receiver side of the OT, deserialising the buffer returned by
// the sender and using this to extract the requested data.
unsigned char *CnC_OT_Output_One_Receiver_0(struct u_v_Pair *c_0, unsigned char inputBit,
								struct otKeyPair *keyPair, struct params_CnC *params, int j)
{
	mpz_t *outputMPZ;
	unsigned char *outputBytes = NULL;
	int curLength, k;


	if(0x00 == inputBit)
	{
		outputMPZ = CnC_OT_Dec(c_0, params, keyPair -> sk);
		outputBytes = convertMPZToBytes(*outputMPZ, &curLength);
	}
	else if(0x01 == params -> crs -> J_set[j])
	{
		outputMPZ = CnC_OT_Dec_Alt(c_0, params, keyPair -> sk, 0x01);
		outputBytes = convertMPZToBytes(*outputMPZ, &curLength);
	}


	return outputBytes;
}


// Perform the second half of the receiver side of the OT, deserialising the buffer returned by
// the sender and using this to extract the requested data.
unsigned char *CnC_OT_Output_One_Receiver_1(struct u_v_Pair *c_1, unsigned char inputBit,
								struct otKeyPair *keyPair, struct params_CnC *params, int j)
{
	mpz_t *outputMPZ;
	unsigned char *outputBytes = NULL;
	int curLength, k;


	if(0x01 == inputBit)
	{
		outputMPZ = CnC_OT_Dec(c_1, params, keyPair -> sk);
		outputBytes = convertMPZToBytes(*outputMPZ, &curLength);
	}
	else if(0x01 == params -> crs -> J_set[j])
	{
		outputMPZ = CnC_OT_Dec_Alt(c_1, params, keyPair -> sk, 0x00);
		outputBytes = convertMPZToBytes(*outputMPZ, &curLength);
	}


	return outputBytes;
}


// Perform the second half of the receiver side of the OT, deserialising the buffer returned by
// the sender and using this to extract the requested data.
void CnC_OT_Output_One_Receiver(struct u_v_Pair *c_0, struct u_v_Pair *c_1,
								struct otKeyPair *keyPair, struct params_CnC *params,
								unsigned char inputBit, int j,
								unsigned char **output_0, unsigned char **output_1)
{
	mpz_t *outputMPZ, *output_j_MPZ;
	unsigned char *outputBytes;
	int curLength, k;


	if(0x00 == inputBit)
	{
		outputMPZ = CnC_OT_Dec(c_0, params, keyPair -> sk);
		*output_0 = convertMPZToBytes(*outputMPZ, &curLength);

		if(0x01 == params -> crs -> J_set[j])
		{
			output_j_MPZ = CnC_OT_Dec_Alt(c_1, params, keyPair -> sk, 0x00);
			*output_1 = convertMPZToBytes(*output_j_MPZ, &curLength);
		}
		else
		{
			*output_1 = NULL;
		}
	}
	else
	{
		outputMPZ = CnC_OT_Dec(c_1, params, keyPair -> sk);
		*output_1 = convertMPZToBytes(*outputMPZ, &curLength);

		if(0x01 == params -> crs -> J_set[j])
		{
			output_j_MPZ = CnC_OT_Dec_Alt(c_0, params, keyPair -> sk, 0x01);
			*output_0 = convertMPZToBytes(*output_j_MPZ, &curLength);
		}
		else
		{
			*output_0 = NULL;
		}
	}
}


// Function used purely to suss out what the heck was going on and why I was failing so hard.
// Turns out I forgot First Year Group Theory...Whoops.
void testStuff(struct params_CnC *params, int j)
{
	mpz_t temp0, temp1, temp2, temp3, temp4, temp5;
	mpz_init(temp0);	mpz_init(temp1);
	mpz_init(temp2);	mpz_init(temp3);
	mpz_init(temp4);	mpz_init(temp5);

	mpz_invert(temp0, params -> y, params -> group -> q);
	mpz_powm(temp1, params -> crs -> h_1_List[j], temp0, params -> group -> p);

	gmp_printf("+ %Zd\n\n", params -> crs -> h_0_List[j]);
	gmp_printf("+ %Zd\n\n", temp1);


	mpz_powm(temp1, params -> crs -> h_0_List[j], params -> y, params -> group -> p);
	mpz_invert(temp2, temp1, params -> group -> p);
	mpz_mul(temp3, temp2, params -> crs -> h_1_List[j]);
	mpz_mod(temp4, temp3, params -> group -> p);

	gmp_printf("- %Zd\n\n", temp4);
}


void test_local_CnC_OT()
{
	struct params_CnC *params_R, *params_S;
	int i, j, k, numInputs = 8, numTests = 128, comp_SecParam = 1024;
	int totalOTs = numInputs * numTests;

	struct otKeyPair **keyPairs_R = (struct otKeyPair **) calloc(numTests, sizeof(struct otKeyPair*));
	struct otKeyPair **keyPairs_S;
	struct u_v_Pair **c_i_Array_S = (struct u_v_Pair **) calloc(2*numTests, sizeof(struct u_v_Pair*));
	struct u_v_Pair **c_i_Array_R;

	unsigned char *inputBytes[numTests][2];
	unsigned char *outputBytes[numTests][2];
	unsigned char *tempChars_0, *tempChars_1;
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
	u_v_index = 0;
	commBuffer = serialise_U_V_Pair_Array(c_i_Array_S, numTests * 2, &bufferOffset);
	c_i_Array_R = deserialise_U_V_Pair_Array(commBuffer, numTests * 2);

	bufferOffset = 0;
	for(i = 0; i < numTests; i ++)
	{
		// CnC_OT_Output_One_Receiver(c_i_Array_R[u_v_index], c_i_Array_R[u_v_index + 1], keyPairs_R[i], params_R, sigmaBit, i, &tempChars_0, &tempChars_1);

		tempChars_0 = CnC_OT_Output_One_Receiver_0(c_i_Array_R[u_v_index + 0], sigmaBit, keyPairs_R[i], params_R, j);
		tempChars_1 = CnC_OT_Output_One_Receiver_1(c_i_Array_R[u_v_index + 1], sigmaBit, keyPairs_R[i], params_R, j);

		outputBytes[i][0] = tempChars_0;
		outputBytes[i][1] = tempChars_1;
		u_v_index += 2;
	}


	// printTrueTestInputs(numTests, inputBytes, sigmaBit);
	// printTrueTestInputs(numTests, outputBytes, sigmaBit);
	testVictory(numTests, inputBytes, outputBytes, sigmaBit, params_R -> crs -> J_set);
}

