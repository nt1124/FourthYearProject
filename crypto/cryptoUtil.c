
#include <string.h>
#include <time.h>
#include "aes.h"


unsigned char *encryptMultiple(unsigned char **keyList, int numKeys, unsigned char *toEncrypt)
{
	unsigned int *RK;
	unsigned char *ciphertext = (unsigned char*) calloc(16, sizeof(unsigned char));
	int i;

	RK = getUintKeySchedule(keyList[0]);
	aes_128_encrypt( ciphertext, toEncrypt, RK );

	for(i = 1; i < numKeys; i ++)
	{
		RK = getUintKeySchedule(keyList[i]);
		aes_128_encrypt( ciphertext, ciphertext, RK );
	}

	return ciphertext;
}


unsigned char *decryptMultiple(unsigned char **keyList, int numKeys, unsigned char *toDecrypt)
{
	unsigned int *encRK, *decRK;
	unsigned char *ciphertext = (unsigned char*) calloc(16, sizeof(unsigned char));
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
