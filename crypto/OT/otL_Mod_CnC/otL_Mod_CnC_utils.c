struct tildeList *initTildeList(int numInputs, mpz_t r_j, struct CRS_CnC_ECC *crs,
								struct eccParams *params, unsigned char inputBit)
{
	struct tildeList *output = (struct tildeList*) calloc(1, sizeof(struct tildeList));
	int i;


	output -> h_tildeList = (struct eccPoint**) calloc(numInputs, sizeof(struct eccPoint*));

	if(0x00 == inputBit)
	{
		output -> g_tilde = windowedScalarPoint(r_j, params -> g, params);
		for(i = 0; i < numInputs; i ++)
		{
			output -> h_tildeList[i] = windowedScalarPoint(r_j, crs -> h_0_List[i], params);
		}
	}
	else
	{
		output -> g_tilde = windowedScalarPoint(r_j, crs -> g_1, params);
		for(i = 0; i < numInputs; i ++)
		{
			output -> h_tildeList[i] = windowedScalarPoint(r_j, crs -> h_1_List[i], params);
		}
	}

	return output;
}


struct tildeCRS *initTildeCRS(int stat_secParam, struct eccParams *params, gmp_randstate_t state)
{
	struct tildeCRS *output = (struct tildeCRS*) calloc(1, sizeof(struct tildeCRS));
	int i;


	output -> r_List = (mpz_t *) calloc(stat_secParam, sizeof(mpz_t));
	for(i = 0; i < stat_secParam; i ++)
	{
		mpz_urandomm(output -> r_List[i], state, params -> n);
	}
}


unsigned char *serialiseTildeList(struct tildeList *listToSerialise, int listLength, int *outputLength)
{
	unsigned char *commBuffer;
	int commBufferLen, hTildesBufferLen = 0, bufferOffset = 0;
	int i;


	for(i = 0; i < listLength; i ++)
	{
		hTildesBufferLen += sizeOfSerial_ECCPoint(listToSerialise -> h_tildeList[i]);
	}


	*outputLength = hTildesBufferLen + sizeOfSerial_ECCPoint(listToSerialise -> g_tilde);
	commBuffer = (unsigned char *) calloc(*outputLength, sizeof(unsigned char));


	serialise_ECC_Point(listToSerialise -> g_tilde, commBuffer, &bufferOffset);
	for(i = 0; i < listLength; i ++)
	{
		serialise_ECC_Point(listToSerialise -> h_tildeList[i], commBuffer, &bufferOffset);
	}


	return commBuffer;
}



struct tildeList *deserialiseTildeList(unsigned char *commBuffer, int listLength, int *inputOffset)
{
	struct tildeList *toReturn = (struct tildeList*) calloc(1, sizeof(struct tildeList));;
	int bufferOffset =  *inputOffset, i;


	toReturn -> h_tildeList = (struct eccPoint **) calloc(listLength, sizeof(struct eccPoint*));

	toReturn -> g_tilde = deserialise_ECC_Point(commBuffer, &bufferOffset);
	for(i = 0; i < listLength; i ++)
	{
		toReturn -> h_tildeList[i] = deserialise_ECC_Point(commBuffer, &bufferOffset);
	}


	return toReturn;
}
