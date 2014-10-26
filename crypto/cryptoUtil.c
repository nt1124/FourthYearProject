
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

	for(i = 1; i < numKeys; i ++)
	{
		RK = getUintKeySchedule(keyList[i]);
		for(j = 0; j < blockCount; j ++)
			aes_128_encrypt( (ciphertext + j*16), (toEncrypt + j*16), RK );
	}

	return ciphertext;
}


unsigned char *decryptMultipleKeys(unsigned char **keyList, int numKeys, unsigned char *toDecrypt, int blockCount)
{
	unsigned int *encRK, *decRK;
	unsigned char *ciphertext = (unsigned char*) calloc(16 * blockCount, sizeof(unsigned char));
	int i;

	encRK = getUintKeySchedule(keyList[0]);
	decRK = decryptionKeySchedule_128(encRK);
	aes_128_decrypt( ciphertext, toDecrypt, decRK );

	for(i = 1; i < numKeys; i ++)
	{
		encRK = getUintKeySchedule(keyList[0]);
		decRK = decryptionKeySchedule_128(encRK);
		aes_128_decrypt( ciphertext, ciphertext, decRK );
	}

	return ciphertext;
}

