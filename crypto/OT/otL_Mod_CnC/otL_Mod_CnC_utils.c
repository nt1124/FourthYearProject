/*
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
*/




unsigned char *serialise_Mod_CTs(struct CnC_OT_Mod_CTs **CTs, int arrayLen, int *outputLength, int keyLength)
{
	unsigned char *commBuffer;
	int i, bufferOffset = sizeof(int), totalLength = sizeof(int);


	for(i = 0; i < arrayLen; i ++)
	{
		totalLength += sizeOfSerial_ECCPoint(CTs[i] -> u_0);
		totalLength += sizeOfSerial_ECCPoint(CTs[i] -> u_1);
	}
	totalLength += (2 * arrayLen * keyLength);


	commBuffer = (unsigned char*) calloc(totalLength, sizeof(unsigned char));

	memcpy(commBuffer, &arrayLen, sizeof(int));
	for(i = 0; i < arrayLen; i ++)
	{
		serialise_ECC_Point(CTs[i] -> u_0, commBuffer, &bufferOffset);
		serialise_ECC_Point(CTs[i] -> u_1, commBuffer, &bufferOffset);

		memcpy(commBuffer + bufferOffset, CTs[i] -> w_0, keyLength);
		bufferOffset += keyLength;
		memcpy(commBuffer + bufferOffset, CTs[i] -> w_1, keyLength);
		bufferOffset += keyLength;
	}


	*outputLength = bufferOffset;

	return commBuffer;
}


struct CnC_OT_Mod_CTs **deserialise_Mod_CTs(unsigned char *commBuffer, int *inputOffset, int keyLength)
{
	struct CnC_OT_Mod_CTs **CTs;
	int i, bufferOffset = *inputOffset, arrayLen;


	memcpy(&arrayLen, commBuffer + bufferOffset, sizeof(int));
	bufferOffset += sizeof(int);

	CTs = (struct CnC_OT_Mod_CTs **) calloc(arrayLen, sizeof(struct CnC_OT_Mod_CTs *));

	for(i = 0; i < arrayLen; i ++)
	{
		CTs[i] = (struct CnC_OT_Mod_CTs *) calloc(arrayLen, sizeof(struct CnC_OT_Mod_CTs));

		CTs[i] -> u_0 = deserialise_ECC_Point(commBuffer, &bufferOffset);
		CTs[i] -> u_1 = deserialise_ECC_Point(commBuffer, &bufferOffset);

		CTs[i] -> w_0 = (unsigned char *) calloc(keyLength, sizeof(unsigned char));
		memcpy(CTs[i] -> w_0, commBuffer + bufferOffset, keyLength);
		bufferOffset += keyLength;

		CTs[i] -> w_1 = (unsigned char *) calloc(keyLength, sizeof(unsigned char));
		memcpy(CTs[i] -> w_1, commBuffer + bufferOffset, keyLength);
		bufferOffset += keyLength;
	}


	*inputOffset = bufferOffset;

	return CTs;
}


unsigned char *serialise_jSet_CheckTildes(struct jSetCheckTildes *toSerialise, int arrayLen, int *outputLength)
{
	unsigned char *commBuffer;
	int i, commBufferLen = sizeof(int);
	int bufferOffset = sizeof(int);


	for(i = 0; i < 2 * arrayLen; i ++)
	{
		commBufferLen += sizeOfSerial_ECCPoint(toSerialise -> h_tildeList[i]);
	}

	commBuffer = (unsigned char *) calloc(commBufferLen, sizeof(unsigned char));

	memcpy(commBuffer, &arrayLen, sizeof(int));
	for(i = 0; i < 2 * arrayLen; i ++)
	{
		serialise_ECC_Point(toSerialise -> h_tildeList[i], commBuffer, &bufferOffset);
	}

	*outputLength = bufferOffset;

	return commBuffer;
}



struct jSetCheckTildes *deserialise_jSet_CheckTildes(unsigned char *commBuffer, int *inputOffset)
{
	struct jSetCheckTildes *output = (struct jSetCheckTildes *) calloc(1, sizeof(struct jSetCheckTildes));
	int i, arrayLen;
	int bufferOffset = *inputOffset;


	memcpy(&arrayLen, commBuffer, sizeof(int));
	bufferOffset += sizeof(int);

	output -> h_tildeList = (struct eccPoint **) calloc(2 * arrayLen, sizeof(struct eccPoint *));

	for(i = 0; i < 2 * arrayLen; i ++)
	{
		output -> h_tildeList[i] = deserialise_ECC_Point(commBuffer, &bufferOffset);
	}


	*inputOffset = bufferOffset;

	return output;
}





unsigned char *serialise_OT_Mod_Check_CTs(struct CnC_OT_Mod_Check_CT **CTs, int arrayLen, int *outputLength, int keyLength)
{
	unsigned char *commBuffer;
	int i, bufferOffset = sizeof(int), totalLength = sizeof(int);


	for(i = 0; i < arrayLen; i ++)
	{
		totalLength += sizeOfSerial_ECCPoint(CTs[i] -> u);
	}
	totalLength += (arrayLen * keyLength);

	commBuffer = (unsigned char*) calloc(totalLength, sizeof(unsigned char));

	memcpy(commBuffer, &arrayLen, sizeof(int));
	for(i = 0; i < arrayLen; i ++)
	{
		serialise_ECC_Point(CTs[i] -> u, commBuffer, &bufferOffset);

		memcpy(commBuffer + bufferOffset, CTs[i] -> w, keyLength);
		bufferOffset += keyLength;
	}


	*outputLength = bufferOffset;

	return commBuffer;
}


struct CnC_OT_Mod_Check_CT **deserialise_OT_Mod_Check_CTs(unsigned char *commBuffer, int *inputOffset, int keyLength)
{
	struct CnC_OT_Mod_Check_CT **CTs;
	int i, bufferOffset = *inputOffset, arrayLen;


	memcpy(&arrayLen, commBuffer + bufferOffset, sizeof(int));
	bufferOffset += sizeof(int);

	CTs = (struct CnC_OT_Mod_Check_CT **) calloc(arrayLen, sizeof(struct CnC_OT_Mod_Check_CT *));

	for(i = 0; i < arrayLen; i ++)
	{
		CTs[i] = (struct CnC_OT_Mod_Check_CT *) calloc(arrayLen, sizeof(struct CnC_OT_Mod_Check_CT));

		CTs[i] -> u = deserialise_ECC_Point(commBuffer, &bufferOffset);

		CTs[i] -> w = (unsigned char *) calloc(keyLength, sizeof(unsigned char));
		memcpy(CTs[i] -> w, commBuffer + bufferOffset, keyLength);
		bufferOffset += keyLength;
	}


	*inputOffset = bufferOffset;

	return CTs;
}






struct ECC_PK *generate_Base_CnC_OT_Mod_PK(struct params_CnC_ECC *params_S, struct tildeList *tildes, int j)
{
	struct ECC_PK *PK = (struct ECC_PK *) calloc(1, sizeof(struct ECC_PK));


	PK -> g = initECC_Point();
	PK -> h = initECC_Point();

	PK -> g_x = copyECC_Point(tildes -> g_tilde);
	PK -> h_x = initECC_Point();

	return PK;
}


void update_CnC_OT_Mod_PK_With_0(struct ECC_PK *PK, struct params_CnC_ECC *params_S, struct tildeList *tildes, int j)
{
	mpz_set(PK -> g -> x, params_S -> params -> g -> x);
	mpz_set(PK -> g -> y, params_S -> params -> g -> y);

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


struct ECC_PK *generateBase_CnC_OT_Mod_CheckPK(params_CnC_ECC *params_S, struct jSetCheckTildes *checkTildes, struct eccPoint *invG1, int j)
{
	struct ECC_PK *PK = (struct ECC_PK *) calloc(1, sizeof(struct ECC_PK));
	int jDoubled = 2 * j;


	PK -> g = copyECC_Point(params_S -> crs -> h_0_List[j]);
	PK -> h = groupOp(params_S -> crs -> h_1_List[j], invG1, params_S -> params);

	PK -> g_x = copyECC_Point(checkTildes -> h_tildeList[jDoubled]);
	PK -> h_x = copyECC_Point(checkTildes -> h_tildeList[jDoubled + 1]);


	return PK;
}