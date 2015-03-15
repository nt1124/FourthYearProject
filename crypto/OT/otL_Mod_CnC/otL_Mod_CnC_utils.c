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





struct ECC_PK *generateFullKey(struct params_CnC_ECC *params_S, struct tildeList *tildes, int j, unsigned char inputBit)
{
	struct ECC_PK *PK = (struct ECC_PK *) calloc(1, sizeof(struct ECC_PK));

	if(0x00 == inputBit)
	{
		PK -> g = copyECC_Point(params_S -> params -> g);
		PK -> h = copyECC_Point(params_S -> crs -> h_0_List[j]);
	}
	else
	{
		PK -> g = copyECC_Point(params_S -> crs -> g_1);
		PK -> h = copyECC_Point(params_S -> crs -> h_1_List[j]);
	}

	PK -> g_x = copyECC_Point(tildes -> g_tilde);
	PK -> h_x = copyECC_Point(tildes -> h_tildeList[j]);

	return PK;
}



struct ECC_PK *generate_Base_CnC_OT_Mod_PK(struct params_CnC_ECC *params_S, struct tildeList *tildes)
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


void freeECC_PK(struct ECC_PK *PK)
{
	clearECC_Point(PK -> g);
	clearECC_Point(PK -> h);
	clearECC_Point(PK -> g_x);
	clearECC_Point(PK -> h_x);

	free(PK);
}