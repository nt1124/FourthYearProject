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

	CT = encDDH(pk, params -> group, M, state);

	return CT;
}


mpz_t *CnC_OT_Dec(struct u_v_Pair *CT, struct params_CnC *params, PVM_OT_SK *sk)
{
	mpz_t *M_Prime = decDDH(sk, params -> group, CT);
	
	return M_Prime;
}


mpz_t *CnC_OT_Dec_Alt(struct u_v_Pair *CT, struct params_CnC *params, PVM_OT_SK *sk)
{
	PVM_OT_SK *sk_z = (PVM_OT_SK *) calloc(1, sizeof(PVM_OT_SK));
	mpz_init(*sk_z);


    mpz_invert(*sk_z, params -> y, params -> group -> p);

	mpz_mul(*sk_z, *sk, *sk_z);

	mpz_t *M_Prime = decDDH(sk_z, params -> group, CT);
	
	return M_Prime;
}


struct params_CnC *setup_CnC_OT_Receiver(int stat_SecParam, struct DDH_Group *group,
							int comp_SecParam, gmp_randstate_t state, 
							unsigned char *bufferToSend, int *bufferLength)
{
	struct params_CnC *params = initParams_CnC(stat_SecParam, state);
	mpz_t alpha;

	// This is ~eight times more than we need, but easier to deal code.
	// Deal with it properly later.
	unsigned char *groupBytes, *CRS_Bytes, tempBytes;
	int i = 0, groupBytesLen, CRS_BytesLen, tempInt;

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

		if( 0x00 == params -> crs -> J_set[i] )
		{
			mpz_add_ui(alpha, alpha, 1);
		}

		mpz_powm(params -> crs -> h_1_List[i], params -> crs -> g_1, alpha, params -> group -> p);
	}


	groupBytes = serialiseDDH_Group(params -> group, &groupBytesLen);
	CRS_Bytes = serialise_CRS(params, &CRS_BytesLen);

	*bufferLength = groupBytesLen + CRS_BytesLen;
	bufferToSend = (unsigned char*)  calloc(*bufferLength, sizeof(unsigned char));

	return params;
}


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
								struct params_CnC *params, gmp_randstate_t *state,
								unsigned char *outputBuffer, int *bufferOffset)
{
	struct otKeyPair *keyPair = keyGen_CnC_OT(params, inputBit, *state, j);


	struct u_v_Pair *c_0 = init_U_V(), *c_1 = init_U_V();
	unsigned char *curBytes;
	int curLength;


	serialiseMPZ(keyPair -> pk -> g, outputBuffer, bufferOffset);

	serialiseMPZ(keyPair -> pk -> h, outputBuffer, bufferOffset);

	return keyPair;
}


// Perform the second half of the receiver side of the OT, deserialising the buffer returned by
// the sender and using this to extract the requested data.
unsigned char *CnC_OT_Output_Uni_One_Receiver(unsigned char *inputBuffer, int *bufferOffset,
											struct otKeyPair *keyPair, struct params_CnC *params,
											unsigned char inputBit, int j,
											int *outputLength, unsigned char *output_j, int *output_jLength)
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

		if(0x01 == params -> crs -> J_set[j])
			output_j_MPZ = CnC_OT_Dec_Alt(c_1, params, keyPair -> sk);
	}
	else
	{
		outputMPZ = CnC_OT_Dec(c_1, params, keyPair -> sk);

		if(0x01 == params -> crs -> J_set[j])
			output_j_MPZ = CnC_OT_Dec_Alt(c_0, params, keyPair -> sk);
	}

	outputBytes = convertMPZToBytes(*outputMPZ, outputLength);
	if(0x01 == params -> crs -> J_set[j])
		output_j = convertMPZToBytes(*output_j_MPZ, output_jLength);


	return outputBytes;
}


void CnC_OT_Transfer_One_Sender(unsigned char *input0Bytes, unsigned char *input1Bytes, int inputLengths,
								struct params_CnC *params, gmp_randstate_t *state,
								unsigned char *inputBuffer, int *inputOffset,
								unsigned char *outputBuffer, int *outputOffset,
								int j)
{
	struct otKeyPair *keyPair = initKeyPair();
	struct u_v_Pair *c_0, *c_1;

	mpz_t *outputMPZ, *tempMPZ;
	mpz_t *input0 = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_t *input1 = (mpz_t*) calloc(1, sizeof(mpz_t));
	unsigned char *curBytes;
	int curLength;

	mpz_init(*input0);
	mpz_init(*input1);


	tempMPZ = deserialiseMPZ(inputBuffer, inputOffset);
	mpz_set(keyPair -> pk -> g, *tempMPZ);
	
	tempMPZ = deserialiseMPZ(inputBuffer, inputOffset);
	mpz_set(keyPair -> pk -> h, *tempMPZ);


	convertBytesToMPZ(input0, input0Bytes, inputLengths);
	convertBytesToMPZ(input1, input1Bytes, inputLengths);


	c_0 = CnC_OT_Enc(*input0, params, *state, keyPair -> pk, 0x00, j);
	c_1 = CnC_OT_Enc(*input1, params, *state, keyPair -> pk, 0x01, j);


	serialise_U_V_pair(c_0, outputBuffer, outputOffset);
	serialise_U_V_pair(c_1, outputBuffer, outputOffset);
}


