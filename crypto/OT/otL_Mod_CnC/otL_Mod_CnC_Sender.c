


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
	int bufferLength = 0;

	commBuffer = receiveBoth(readSocket, bufferLength);
	params_S = setup_CnC_OT_Mod_Sender(commBuffer);
	free(commBuffer);

	ZKPoK_DL_Verifier(writeSocket, readSocket, params_S -> params,
					params_S -> params -> g, params_S -> crs -> g_1, state);

	return params_S;
}


struct ECC_PK *generate_Base_CnC_OT_Mod_PK(struct params_CnC_ECC *params_S, struct tildeList *tildes, int j)
{
	struct ECC_PK *PK = (struct ECC_PK *) calloc(1, sizeof(struct ECC_PK));


	PK -> g = initECC_Point();
	// PK -> g = copyECC_Point(params_S -> params -> g);
	PK -> h = initECC_Point();
	// PK -> h = copyECC_Point(params_S -> crs -> h_0_List[j]);

	PK -> g_x = copyECC_Point(tildes -> g_tilde);
	PK -> h_x = initECC_Point();
	// PK -> h_x = copyECC_Point(tildes -> h_tildeList[j]);

	return PK;
}


void update_CnC_OT_Mod_PK_With_0(struct ECC_PK *PK, struct params_CnC_ECC *params_S, struct tildeList *tildes, int j)
{
	mpz_set(PK -> g -> x, params_S -> crs -> g_1 -> x);
	mpz_set(PK -> g -> y, params_S -> crs -> g_1 -> y);

	mpz_set(PK -> h -> x, params_S -> crs -> h_0_List[j] -> x);
	mpz_set(PK -> h -> y, params_S -> crs -> h_0_List[j] -> y);

	mpz_set(PK -> h_x -> x, tildes -> h_tildeList[j] -> x);
	mpz_set(PK -> h_x -> y, tildes -> h_tildeList[j] -> y);
}


void update_CnC_OT_Mod_PK_With_1(struct ECC_PK *PK, struct params_CnC_ECC *params_S, int j)
{
	mpz_set(PK -> g -> x, params_S -> crs -> g_1 -> x);
	mpz_set(PK -> g -> y, params_S -> crs -> g_1 -> y);

	mpz_set(PK -> h -> x, params_S -> crs -> h_1_List[j] -> x);
	mpz_set(PK -> h -> y, params_S -> crs -> h_1_List[j] -> y);
}


struct CnC_OT_Mod_CTs **CnC_OT_Mod_Enc_i(struct params_CnC_ECC *params_S, struct tildeList *tildes,
								unsigned char *M,
								gmp_randstate_t state)
{
	struct CnC_OT_Mod_CTs **CTs;
	struct ECC_PK *PK;
	struct u_v_Pair_ECC *tempCT;
	unsigned char *v_xBytes, *hashedV_x;

	unsigned char *commBuffer;

	const int stat_SecParam = params_S -> crs -> stat_SecParam, msgLength = 16;
	int j, tempLength;


	CTs = (struct CnC_OT_Mod_CTs **) calloc(stat_SecParam, sizeof(struct CnC_OT_Mod_CTs *));
	PK = generate_Base_CnC_OT_Mod_PK(params_S, tildes, j);


	for(j = 0; j < stat_SecParam; j ++)
	{
		CTs[j] = (struct CnC_OT_Mod_CTs *) calloc(1, sizeof(struct CnC_OT_Mod_CTs ));

		update_CnC_OT_Mod_PK_With_0(PK, params_S, tildes, j);
		tempCT = randomiseDDH_ECC(PK, params_S -> params, state);
		v_xBytes = convertMPZToBytes(tempCT -> v -> x, &tempLength);
		hashedV_x = sha256_full(v_xBytes, tempLength);

		CTs[j] -> u_0 = tempCT -> u;
		CTs[j] -> w_0 = XOR_TwoStrings(hashedV_x, M, msgLength);

		clearECC_Point(tempCT -> v);
		free(hashedV_x);
		free(v_xBytes);
		free(tempCT);


		update_CnC_OT_Mod_PK_With_1(PK, params_S, j);
		tempCT = randomiseDDH_ECC(PK, params_S -> params, state);
		v_xBytes = convertMPZToBytes(tempCT -> v -> x, &tempLength);
		hashedV_x = sha256_full(v_xBytes, tempLength);

		CTs[j] -> u_1 = tempCT -> u;
		CTs[j] -> w_1 = XOR_TwoStrings(hashedV_x, M, msgLength);

		clearECC_Point(tempCT -> v);
		free(hashedV_x);
		free(v_xBytes);
		free(tempCT);
	}


	return CTs;
}
