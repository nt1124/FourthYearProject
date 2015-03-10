unsigned char *generate_Mod_J_Set(int stat_SecParam)
{
	unsigned char *J_set = (unsigned char *) calloc(stat_SecParam, sizeof(int));
	int i = 0, tempInt = 0;


	for(i = 0; i < stat_SecParam; i ++)
	{
	
		tempInt = rand() % 2;
		if(1 == tempInt)
		{
			J_set[tempInt] = 0x01;
		}
	}


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

	params -> crs -> g_1 = windowedScalarPoint(params -> y, params -> params -> g, params -> params);

	for(i = 0; i < stat_SecParam; i ++)
	{	
		do
		{
			mpz_urandomm(alpha, state, params -> params -> n);
		} while( 0 == mpz_cmp_ui( alpha, 0) );

		params -> crs -> h_0_List[i] = windowedScalarPoint(alpha, params -> params -> g, params -> params);

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
													int stat_SecParam,	int comp_SecParam, gmp_randstate_t state)
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



struct tildeCRS *setup_Receiver_tildeCRS(struct params_CnC_ECC *params_R, int numInputs,
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



unsigned char *transfer_1_CnC_OT_Mod_Receiver(struct params_CnC_ECC *params_R, gmp_randstate_t state)
{

}







