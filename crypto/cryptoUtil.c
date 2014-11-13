
#include <string.h>
#include <time.h>
#include "aes.h"


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



