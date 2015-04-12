// Initialise the dec side of the params
struct CRS_CnC_ECC *initCRS_CnC_ECC(int stat_SecParam)
{
	struct CRS_CnC_ECC *crs = (struct CRS_CnC_ECC*) calloc(1, sizeof(struct CRS_CnC_ECC));
	int i;


	crs -> g_1 = initECC_Point();

	crs -> stat_SecParam = stat_SecParam;

	crs -> J_set = NULL;
	crs -> alphas_List = (mpz_t *) calloc(crs -> stat_SecParam, sizeof(mpz_t));
	crs -> h_0_List = (struct eccPoint **) calloc(crs -> stat_SecParam, sizeof(struct eccPoint*));
	crs -> h_1_List = (struct eccPoint **) calloc(crs -> stat_SecParam, sizeof(struct eccPoint*));

	for(i = 0; i < crs -> stat_SecParam; i ++)
	{
		mpz_init(crs -> alphas_List[i]);
		crs -> h_0_List[i] = initECC_Point();
		crs -> h_1_List[i] = initECC_Point();
	}


	return crs;
}

// Initialise the dec side of the params
struct params_CnC_ECC *initParams_CnC_ECC(int stat_SecParam, int comp_SecParam, gmp_randstate_t state)
{
	struct params_CnC_ECC *params = (struct params_CnC_ECC*) calloc(1, sizeof(struct params_CnC_ECC));

	params -> crs = initCRS_CnC_ECC(stat_SecParam);
	// params -> group = generateGroup(comp_SecParam, state);
	params -> params = initBrainpool_256_Curve();

	mpz_init(params -> y);

	return params;
}

/*
unsigned char *generateJ_Set(int stat_SecParam)
{
	unsigned char *J_set = (unsigned char *) calloc(stat_SecParam, sizeof(int));
	int i = 0, tempInt;

	while(i < stat_SecParam / 2)
	{
		tempInt = rand() % stat_SecParam;
		if(0x00 == J_set[tempInt])
		{
			J_set[tempInt] = 0x01;
			i ++;
		}
	}

	return J_set;
}
*/

unsigned char *serialise_CRS_ECC(struct params_CnC_ECC *params, int *bufferOffset)
{
	unsigned char *output;
	int i, totalLength = 0, tempInt;
	*bufferOffset = sizeof(int);


	totalLength = 0;

	for(i = 0 ; i < params -> crs -> stat_SecParam; i ++)
	{
		totalLength += sizeOfSerial_ECCPoint(params -> crs -> h_0_List[i]);
		totalLength += sizeOfSerial_ECCPoint(params -> crs -> h_1_List[i]);
	}

	tempInt = sizeof(int) + sizeOfSerial_ECCPoint(params -> crs -> g_1);

	output = (unsigned char *) calloc(tempInt + totalLength, sizeof(unsigned char));

	memcpy(output, &(params -> crs -> stat_SecParam), sizeof(int));
	serialise_ECC_Point(params -> crs -> g_1, output, bufferOffset);

	for(i = 0; i < params -> crs -> stat_SecParam; i ++)
	{
		serialise_ECC_Point(params -> crs -> h_0_List[i], output, bufferOffset);
	}
	for(i = 0; i < params -> crs -> stat_SecParam; i ++)
	{
		serialise_ECC_Point(params -> crs -> h_1_List[i], output, bufferOffset);
	}

	return output;
}


struct CRS_CnC_ECC *deserialise_CRS_ECC(unsigned char *inputBuffer, int *bufferOffset)
{
	struct CRS_CnC_ECC *crs;
	int i, tempInt;


	memcpy(&tempInt, inputBuffer + *bufferOffset, sizeof(int));
	crs = initCRS_CnC_ECC(tempInt);
	*bufferOffset += sizeof(int);


	crs -> g_1 = deserialise_ECC_Point(inputBuffer, bufferOffset);

	for(i = 0; i < crs -> stat_SecParam; i ++)
	{
		crs -> h_0_List[i] = deserialise_ECC_Point(inputBuffer, bufferOffset);
	}
	for(i = 0; i < crs -> stat_SecParam; i ++)
	{
		crs -> h_1_List[i] = deserialise_ECC_Point(inputBuffer, bufferOffset);
	}

	return crs;
}


unsigned char *serialiseParams_CnC_ECC(struct params_CnC_ECC *params, int *outputLength)
{
	unsigned char *paramsBytes, *CRS_Bytes, *outputBytes;
	int i = 0, paramsBytesLen = 0, CRS_BytesLen = 0, tempInt = 0;

	paramsBytes = serialiseECC_Params(params -> params, &paramsBytesLen);
	CRS_Bytes = serialise_CRS_ECC(params, &CRS_BytesLen);

	*outputLength = paramsBytesLen + CRS_BytesLen;

	outputBytes = (unsigned char*)  calloc(*outputLength, sizeof(unsigned char));


	memcpy(outputBytes + i, paramsBytes, paramsBytesLen);
	i += paramsBytesLen;

	memcpy(outputBytes + i, CRS_Bytes, CRS_BytesLen);
	i += CRS_BytesLen;

	free(paramsBytes);
	free(CRS_Bytes);

	return outputBytes;
}


void freeParams_CnC_ECC(struct params_CnC_ECC *params)
{
	int i;

	for(i = 0; i < params -> crs -> stat_SecParam; i ++)
	{
		clearECC_Point(params -> crs -> h_0_List[i]);
		clearECC_Point(params -> crs -> h_1_List[i]);
		mpz_clear(params -> crs -> alphas_List[i]);
	}
	clearECC_Point(params -> crs -> g_1);
	free(params -> crs -> h_0_List);
	free(params -> crs -> h_1_List);
	free(params -> crs -> alphas_List);
	free(params -> crs -> J_set);

	freeECC_Params(params -> params);

	mpz_clear(params -> y);

	free(params);
}
