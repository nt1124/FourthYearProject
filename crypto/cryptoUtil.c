unsigned char getBitFromCharArray(unsigned char *input, int indexToGet)
{
	int i = indexToGet / 8;
	int j = indexToGet % 8;

	return (input[i] & (0x01 << j)) >> j;
}



unsigned char *encryptMultipleKeys(unsigned char **keyList, int numKeys, unsigned char *toEncrypt, int blockCount)
{
	unsigned int *RK;
	unsigned char *ciphertext = (unsigned char*) calloc(16 * blockCount, sizeof(unsigned char));
	int i, j;

	RK = getUintKeySchedule(keyList[0]);
	for(j = 0; j < blockCount; j ++)
		aes_128_encrypt( (ciphertext + j*16), (toEncrypt + j*16), RK );
	free(RK);

	for(i = 1; i < numKeys; i ++)
	{
		RK = getUintKeySchedule(keyList[i]);
		for(j = 0; j < blockCount; j ++)
			aes_128_encrypt( (ciphertext + j*16), (ciphertext + j*16), RK );
		free(RK);
	}

	return ciphertext;
}


unsigned char *decryptMultipleKeys(unsigned char **keyList, int numKeys, unsigned char *toDecrypt, int blockCount)
{
	unsigned int *encRK, *decRK;
	unsigned char *plaintext = (unsigned char*) calloc(16 * blockCount, sizeof(unsigned char));
	int i, j;

	encRK = getUintKeySchedule(keyList[0]);
	decRK = decryptionKeySchedule_128(encRK);	

	free(encRK);
	for(j = 0; j < blockCount; j ++)
	{
		//To Decrypt is null.
		aes_128_decrypt( (toDecrypt + j*16), (plaintext + j*16), decRK );
	}
	free(decRK);

	for(i = 1; i < numKeys; i ++)
	{
		encRK = getUintKeySchedule(keyList[i]);
		decRK = decryptionKeySchedule_128(encRK);
		free(encRK);
		for(j = 0; j < blockCount; j ++)
		{
			aes_128_decrypt( (plaintext + j*16), (plaintext + j*16), decRK );
		}
		free(decRK);
	}

	return plaintext;
}

void testAES()
{
	unsigned char *encKeyList[3], *decKeyList[3];
	unsigned char *message = generateRandBytes(32, 32);
	unsigned char *ciphertext, *decMessage;
	int i;

	for(i = 0; i < 3; i ++)
	{
		encKeyList[i] = generateRandBytes(16, 16);
		decKeyList[2-i] = (unsigned char*) calloc(16, sizeof(unsigned char));
		memcpy(decKeyList[2-i], encKeyList[i], 16);
	}

	ciphertext = encryptMultipleKeys(encKeyList, 3, message, 2);
	decMessage = decryptMultipleKeys(decKeyList, 3, ciphertext, 2);

	for(i = 0; i < 32; i ++)
		printf("%02X", message[i]);
	printf("\n");

	for(i = 0; i < 32; i ++)
		printf("%02X", ciphertext[i]);
	printf("\n");

	for(i = 0; i < 32; i ++)
		printf("%02X", decMessage[i]);
	printf("\n");
}


void testAES_Zeroed()
{
	unsigned char *encKeyList[1];
	unsigned char *message = (unsigned char*) calloc(16, sizeof(unsigned char));
	unsigned char *ciphertext;
	int i;

	encKeyList[0] = (unsigned char*) calloc(16, sizeof(unsigned char));
	// memset(encKeyList[0], 0xFF, 16);
	ciphertext = encryptMultipleKeys(encKeyList, 1, message, 1);


	printf("  Correct output as Hex: ");
	for(i = 0; i < 16; i ++)
		printf("%02X", ciphertext[i]);
	printf("\n");
}




unsigned char *getDataAsString(const char *filepath)
{
	unsigned char *output = (unsigned char*) calloc(16, sizeof(unsigned char));
	FILE *file = fopen ( filepath, "r" );
	int strIndex, i = 0, j = 0, k = 0;
	char line [ 512 ];


	if ( file != NULL )
	{
		while ( fgets ( line, sizeof line, file ) != NULL && 127 >= i )
		{
			strIndex = 0;

			while( ' ' != line[strIndex++] ){}
			while( ' ' != line[strIndex++] ){}


			if( '1' == line[strIndex] )
			{
				output[j] = output[j] ^ (0x01 << (7 - k));
			}

			k ++;
			if(8 == k)
			{
				k = 0;
				j ++;
			}
			i ++;
		}
		fclose ( file );
	}

	return output;
}


void testAES_FromRandom()
{
	unsigned char **encKeyList = (unsigned char**) calloc(1, sizeof(unsigned char*));
	unsigned char *message = getDataAsString("inputs/randomAES.builder.input"); 
	unsigned char *ciphertext;
	int i;


	encKeyList[0] = getDataAsString("inputs/randomAES.executor.input");

	ciphertext = encryptMultipleKeys(encKeyList, 1, message, 1);


	printf("  Correct output as Hex: ");
	for(i = 0; i < 16; i ++)
		printf("%02X", ciphertext[i]);
	printf("\n");
}


unsigned char *XOR_TwoStrings(unsigned char *x_1, unsigned char *x_2, int length)
{
	unsigned char *output = (unsigned char *) calloc(length, sizeof(unsigned char));
	int i;

	for(i = 0; i < length; i ++)
	{
		output[i] = x_1[i] ^ x_2[i];
		// printf("%02X  =  %02X  %02X\n", output[i], x_1[i], x_2[i]);
	}

	// printf("\n");

	return output;
}


unsigned char *XOR_TwoStringsDiffLength(unsigned char *x_1, unsigned char *x_2, int length1, int length2)
{
	unsigned char *output, *lowerX, *higherX;
	int i, lowerLength, higherLength;


	if(length1 <= length2)
	{
		lowerLength = length1;
		higherLength = length2;
		lowerX = x_1;
		higherX = x_2;
	}
	else
	{
		lowerLength = length2;
		higherLength = length1;
		lowerX = x_2;
		higherX = x_1;
	}


	output = (unsigned char *) calloc(higherLength, sizeof(unsigned char));

	for(i = 0; i < lowerLength; i ++)
	{
		output[i] = x_1[i] ^ x_2[i];
	}
	for(; i < higherLength; i ++)
	{
		output[i] = higherX[i];
	}


	return output;
}


unsigned char *XOR_StringsWithMinLen(unsigned char *x_1, unsigned char *x_2,
									int length1, int length2, int minLength)
{
	unsigned char *output, *lowerX, *higherX;
	int i, lowerLength, higherLength;


	if(length1 <= length2)
	{
		lowerLength = length1;
		higherLength = length2;
		lowerX = x_1;
		higherX = x_2;
	}
	else
	{
		lowerLength = length2;
		higherLength = length1;
		lowerX = x_2;
		higherX = x_1;
	}

	if(minLength <= higherLength)
	{
		output = (unsigned char *) calloc(higherLength, sizeof(unsigned char));
	}
	else
	{
		output = (unsigned char *) calloc(minLength, sizeof(unsigned char));
	}

	for(i = 0; i < lowerLength; i ++)
	{
		output[i] = x_1[i] ^ x_2[i];
	}
	for(; i < higherLength; i ++)
	{
		output[i] = higherX[i];
	}


	return output;
}