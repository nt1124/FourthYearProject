
int sizeOf_C_Box_Array(struct elgamal_commit_box **c, int numC_Boxes)
{
	int i, totalLength = 0;

	for(i = 0; i < numC_Boxes; i ++)
	{
		totalLength += ( sizeOfSerial_ECCPoint(c[i] -> u) + sizeOfSerial_ECCPoint(c[i] -> v) );
	}

	return totalLength;
}


int sizeOf_K_Box_Array(struct elgamal_commit_key **k, int msgLength, int numK_Boxes)
{
	int i, totalLength = sizeof(int) * numK_Boxes;

	for(i = 0; i < numK_Boxes; i ++)
	{
		totalLength += sizeof(mp_limb_t) * mpz_size(k[i] -> r);
		totalLength += msgLength;
	}

	return totalLength;
}


void serialise_elgamal_Cbox(struct elgamal_commit_box *c, unsigned char *outputBuffer, int *bufferOffset)
{
	unsigned char *curBytes;
	int curLength, curOffset = *bufferOffset;

	serialise_ECC_Point(c -> u, outputBuffer, &curOffset);
	serialise_ECC_Point(c -> v, outputBuffer, &curOffset);

	*bufferOffset = curOffset;
}


struct elgamal_commit_box *deserialise_elgamal_Cbox(unsigned char *inputBuffer, int *bufferOffset)
{
	struct elgamal_commit_box *c = init_commit_box();
	int curLength, curOffset = *bufferOffset;


	c -> u = deserialise_ECC_Point(inputBuffer, &curOffset);
	c -> v = deserialise_ECC_Point(inputBuffer, &curOffset);


	*bufferOffset = curOffset;

	return c;
}


void serialise_elgamal_Kbox(struct elgamal_commit_key *k, int msgLength, unsigned char *outputBuffer, int *bufferOffset)
{
	unsigned char *curBytes;
	int curLength, curOffset = *bufferOffset;

	curBytes = convertMPZToBytes(k -> r, &curLength);
	memcpy(outputBuffer + curOffset, &curLength, sizeof(int));
	curOffset += sizeof(int);
	memcpy(outputBuffer + curOffset, curBytes, curLength);
	curOffset += curLength;

	// serialise_ECC_Point(k -> x, outputBuffer, &curOffset);
	memcpy(outputBuffer + curOffset, k -> x, msgLength);
	curOffset += msgLength;



	*bufferOffset = curOffset;
}


struct elgamal_commit_key *deserialise_elgamal_Kbox(unsigned char *inputBuffer, int msgLength, int *bufferOffset)
{
	struct elgamal_commit_key *k = init_commit_key();
	mpz_t *temp = (mpz_t*) calloc(1, sizeof(mpz_t));
	int curLength, curOffset = *bufferOffset;

	mpz_init(*temp);


	memcpy(&curLength, inputBuffer + curOffset, sizeof(int));
	curOffset += sizeof(int);
	convertBytesToMPZ(temp, inputBuffer + curOffset, curLength);
	curOffset += curLength;
	mpz_set(k -> r, *temp);

	k -> x = (unsigned char*) calloc(msgLength, sizeof(unsigned char));
	memcpy(k -> x, inputBuffer + curOffset, msgLength);
	curOffset += msgLength;


	*bufferOffset = curOffset;

	return k;
}


unsigned char *serialiseAllC_Boxes_1D(struct elgamal_commit_box **c, int numC_Boxes, int *totalLength)
{
	unsigned char *outputBuffer;
	int outputLength, i, tempOffset = 0;


	outputLength = sizeOf_C_Box_Array(c, numC_Boxes);
	outputBuffer = (unsigned char *) calloc(outputLength, sizeof(unsigned char));

	for(i = 0 ; i < numC_Boxes; i ++)
	{
		serialise_elgamal_Cbox(c[i], outputBuffer, &tempOffset);
	}


	*totalLength = tempOffset;

	return outputBuffer;
}


struct elgamal_commit_box **deserialiseAllC_Boxes_1D(unsigned char *inputBuffer, int numC_Boxes, int *bufferOffset)
{
	struct elgamal_commit_box **c = (struct elgamal_commit_box **) calloc(numC_Boxes, sizeof(struct elgamal_commit_box *));
	int outputLength, i, tempOffset = *bufferOffset;


	for(i = 0 ; i < numC_Boxes; i ++)
	{
		c[i] = deserialise_elgamal_Cbox(inputBuffer, &tempOffset);
	}


	*bufferOffset = tempOffset;

	return c;
}


unsigned char *serialiseAllC_Boxes_2D(struct elgamal_commit_box ***c, int iMax, int jMax, int *totalLength)
{
	unsigned char *outputBuffer;
	int outputLength = 0, i, j, tempOffset = 0;

	for(i = 0; i < iMax; i ++)
	{
		outputLength += sizeOf_C_Box_Array(c[i], jMax);
	}

	outputBuffer = (unsigned char *) calloc(outputLength, sizeof(unsigned char));

	for(i = 0; i < iMax; i ++)
	{
		for(j = 0 ; j < jMax; j ++)
		{
			serialise_elgamal_Cbox(c[i][j], outputBuffer, &tempOffset);
		}
	}


	*totalLength = tempOffset;

	return outputBuffer;
}


struct elgamal_commit_box ***deserialiseAllC_Boxes_2D(unsigned char *inputBuffer, int iMax, int jMax, int *bufferOffset)
{
	struct elgamal_commit_box ***c = (struct elgamal_commit_box ***) calloc(iMax, sizeof(struct elgamal_commit_box **));
	int outputLength, i, j, tempOffset = *bufferOffset;


	for(i = 0 ; i < iMax; i ++)
	{
		c[i] = (struct elgamal_commit_box **) calloc(jMax, sizeof(struct elgamal_commit_box *));
		for(j = 0 ; j < jMax; j ++)
		{
			c[i][j] = deserialise_elgamal_Cbox(inputBuffer, &tempOffset);
		}
	}


	*bufferOffset = tempOffset;

	return c;
}








unsigned char *serialiseAllK_Boxes_1D(struct elgamal_commit_key **k, int msgLength, int numK_Boxes, int *totalLength)
{
	unsigned char *outputBuffer;
	int outputLength, i, tempOffset = 0;


	outputLength = sizeOf_K_Box_Array(k, msgLength, numK_Boxes);
	outputBuffer = (unsigned char *) calloc(outputLength, sizeof(unsigned char));

	for(i = 0 ; i < numK_Boxes; i ++)
	{
		serialise_elgamal_Kbox(k[i], msgLength, outputBuffer, &tempOffset);
	}


	*totalLength = tempOffset;

	return outputBuffer;
}


struct elgamal_commit_key **deserialiseAllK_Boxes_1D(unsigned char *inputBuffer, int msgLength, int numK_Boxes, int *bufferOffset)
{
	struct elgamal_commit_key **k = (struct elgamal_commit_key **) calloc(numK_Boxes, sizeof(struct elgamal_commit_key *));
	int outputLength, i, tempOffset = *bufferOffset;


	for(i = 0 ; i < numK_Boxes; i ++)
	{
		k[i] = deserialise_elgamal_Kbox(inputBuffer, msgLength, &tempOffset);
	}

	*bufferOffset = tempOffset;

	return k;
}

