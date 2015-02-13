// Initialise the dec side of the params
struct CRS_CnC *initCRS_CnC(int stat_SecParam)
{
	struct CRS_CnC *crs = (struct CRS_CnC*) calloc(1, sizeof(struct CRS_CnC));
	int i;

	mpz_init(crs -> g_1);

	crs -> stat_SecParam = stat_SecParam;

	crs -> J_set = NULL;
	crs -> h_0_List = (mpz_t *) calloc(crs -> stat_SecParam, sizeof(mpz_t));
	crs -> h_1_List = (mpz_t *) calloc(crs -> stat_SecParam, sizeof(mpz_t));

	for(i = 0; i < crs -> stat_SecParam; i ++)
	{
		mpz_init(crs -> h_0_List[i]);
		mpz_init(crs -> h_1_List[i]);
	}

	return crs;
}

// Initialise the dec side of the params
struct params_CnC *initParams_CnC(int stat_SecParam, int comp_SecParam, gmp_randstate_t state)
{
	struct params_CnC *params = (struct params_CnC*) calloc(1, sizeof(struct params_CnC));

	params -> crs = initCRS_CnC(stat_SecParam);
	params -> group = generateGroup(comp_SecParam, state);

	mpz_init(params -> y);

	return params;
}


unsigned char *serialise_CRS(struct params_CnC *params, int *bufferOffset)
{
	unsigned char *output;
	int i, totalLength = 0, tempInt;
	*bufferOffset = sizeof(int);


	totalLength = ( sizeof(mp_limb_t) * mpz_size(params -> crs -> g_1) );

	for(i = 0 ; i < params -> crs -> stat_SecParam; i ++)
	{
		totalLength += ( sizeof(mp_limb_t) * mpz_size(params -> crs -> h_0_List[i]) );
		totalLength += ( sizeof(mp_limb_t) * mpz_size(params -> crs -> h_1_List[i]) );
	}

	tempInt = (2 * params -> crs -> stat_SecParam) + 2;
	tempInt *= sizeof(int);

	output = (unsigned char *) calloc(tempInt + totalLength, sizeof(unsigned char));

	memcpy(output, &(params -> crs -> stat_SecParam), sizeof(int));
	serialiseMPZ(params -> crs -> g_1, output, bufferOffset);
	for(i = 0; i < params -> crs -> stat_SecParam; i ++)
	{
		serialiseMPZ(params -> crs -> h_0_List[i], output, bufferOffset);
	}
	for(i = 0; i < params -> crs -> stat_SecParam; i ++)
	{
		serialiseMPZ(params -> crs -> h_1_List[i], output, bufferOffset);
	}

	return output;
}

struct CRS_CnC *deserialise_CRS(unsigned char *inputBuffer, int *bufferOffset)
{
	struct CRS_CnC *crs;
	int i, tempInt;
	mpz_t *temp = (mpz_t*) calloc(1, sizeof(mpz_t));

	mpz_init(*temp);


	memcpy(&tempInt, inputBuffer + *bufferOffset, sizeof(int));
	crs = initCRS_CnC(tempInt);
	*bufferOffset += sizeof(int);

	temp = deserialiseMPZ(inputBuffer, bufferOffset);
	mpz_set(crs -> g_1, *temp);

	for(i = 0; i < crs -> stat_SecParam; i ++)
	{
		temp = deserialiseMPZ(inputBuffer, bufferOffset);
		mpz_set(crs -> h_0_List[i], *temp);
	}
	for(i = 0; i < crs -> stat_SecParam; i ++)
	{
		temp = deserialiseMPZ(inputBuffer, bufferOffset);
		mpz_set(crs -> h_1_List[i], *temp);
	}

	return crs;
}

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


unsigned char *serialiseParams_CnC(struct params_CnC *params, int *outputLength)
{
	unsigned char *groupBytes, *CRS_Bytes, *outputBytes;
	int i = 0, groupBytesLen = 0, CRS_BytesLen = 0, tempInt = 0;

	groupBytes = serialiseDDH_Group(params -> group, &groupBytesLen);
	CRS_Bytes = serialise_CRS(params, &CRS_BytesLen);

	*outputLength = groupBytesLen + CRS_BytesLen;

	outputBytes = (unsigned char*)  calloc(*outputLength, sizeof(unsigned char));


	memcpy(outputBytes + i, groupBytes, groupBytesLen);
	i += groupBytesLen;

	memcpy(outputBytes + i, CRS_Bytes, CRS_BytesLen);
	i += CRS_BytesLen;

	free(groupBytes);
	free(CRS_Bytes);

	return outputBytes;
}