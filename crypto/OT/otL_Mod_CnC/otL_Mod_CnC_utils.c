struct tildeList *initTildeList(int stat_secParam, mpz_t r_i, struct CRS_CnC_ECC *crs,
								struct eccParams *params, unsigned char inputBit)
{
	struct tildeList *output = (struct tildeList*) calloc(1, sizeof(struct tildeList));
	int i;


	output -> h_tildeList = (struct eccPoint**) calloc(stat_secParam, sizeof(struct eccPoint*));

	if(0x00 == inputBit)
	{
		output -> g_tilde = windowedScalarPoint(r_i, params -> g, params);
		for(i = 0; i < stat_secParam; i ++)
		{
			output -> h_tildeList[i] = windowedScalarPoint(r_i, crs -> h_0_List[i], params);
		}
	}
	else
	{
		output -> g_tilde = windowedScalarPoint(r_i, crs -> g_1, params);
		for(i = 0; i < stat_secParam; i ++)
		{
			output -> h_tildeList[i] = windowedScalarPoint(r_i, crs -> h_1_List[i], params);
		}
	}

	return output;
}


struct tildeCRS *initTildeCRS(int numInput, struct eccParams *params, gmp_randstate_t state)
{
	struct tildeCRS *output = (struct tildeCRS*) calloc(1, sizeof(struct tildeCRS));
	int i;


	output -> r_List = (mpz_t *) calloc(numInput, sizeof(mpz_t));
	for(i = 0; i < numInput; i ++)
	{
		mpz_urandomm(output -> r_List[i], state, params -> n);
	}


	return output;
}


int sizeOfSerialTildeList(struct tildeList *listToSerialise, int listLength)
{
	int hTildesBufferLen = 0, i;


	hTildesBufferLen = sizeOfSerial_ECCPoint(listToSerialise -> g_tilde);
	for(i = 0; i < listLength; i ++)
	{
		hTildesBufferLen += sizeOfSerial_ECCPoint(listToSerialise -> h_tildeList[i]);
	}


	return hTildesBufferLen;
}


void serialiseTildeList_Alt(struct tildeList *listToSerialise, unsigned char *commBuffer,
							int listLength, int *outputOffset)
{
	int bufferOffset = *outputOffset;
	int i;


	serialise_ECC_Point(listToSerialise -> g_tilde, commBuffer, &bufferOffset);
	for(i = 0; i < listLength; i ++)
	{
		serialise_ECC_Point(listToSerialise -> h_tildeList[i], commBuffer, &bufferOffset);
	}


	*outputOffset = bufferOffset;
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
	struct tildeList *toReturn = (struct tildeList*) calloc(1, sizeof(struct tildeList));
	int bufferOffset = *inputOffset, i;


	toReturn -> h_tildeList = (struct eccPoint **) calloc(listLength, sizeof(struct eccPoint*));

	toReturn -> g_tilde = deserialise_ECC_Point(commBuffer, &bufferOffset);
	for(i = 0; i < listLength; i ++)
	{
		toReturn -> h_tildeList[i] = deserialise_ECC_Point(commBuffer, &bufferOffset);
	}


	*inputOffset = bufferOffset;

	return toReturn;
}


unsigned char *serialiseTildeCRS(struct tildeCRS *toSerialise, int numLists, int listLength, int *outputLength)
{
	unsigned char *commBuffer;
	int bufferOffset = 0, i, totalLength = 0;


	for(i = 0; i < numLists; i ++)
	{
		totalLength += sizeOfSerialTildeList(toSerialise -> lists[i], listLength);
	}

	commBuffer = (unsigned char*) calloc(totalLength, sizeof(unsigned char));
	for(i = 0; i < numLists; i ++)
	{
		serialiseTildeList_Alt(toSerialise -> lists[i], commBuffer, listLength, &bufferOffset);
	}


	*outputLength = bufferOffset;

	return commBuffer;
}



struct tildeCRS *deserialiseTildeCRS(unsigned char *commBuffer, int numLists, int listLength, int *inputOffset)
{
	struct tildeCRS *toReturn = (struct tildeCRS *) calloc(1, sizeof(struct tildeCRS));
	int bufferOffset = *inputOffset, i;

	toReturn -> lists = (struct tildeList**) calloc(numLists, sizeof(struct tildeList*));


	for(i = 0; i < numLists; i ++)
	{
		toReturn -> lists[i] = deserialiseTildeList(commBuffer, listLength, &bufferOffset);
	}


	*inputOffset = bufferOffset;

	return toReturn;
}



