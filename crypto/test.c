#include "aes.h"
#include <stdio.h>
#include <stdlib.h>
#include <openssl/aes.h>



int aesTesting()
{
	printf( "CPU capable?  %d\n", Check_CPU_support_AES() );

	int intKey = 2324253, intMessage = 123546236, i;

	octet* tempKey = new octet[16];
	octet* tempMsg = new octet[16];
	octet* tempOut = new octet[16];

	tempMsg = generateRandBytes(16);
	tempKey = generateRandBytes(16);

	uint *RK = new uint[44];
	//aes_schedule( 128, 10, tempKey, RK );
	RK = getUintKeySchedule(tempKey);
	octet* C = new octet[16];

	printf("Message\n");
	for(i = 0; i < 16; i ++)
	{
		printf("%02X", tempMsg[i]);
	}

	aes_128_encrypt(C, tempMsg, RK);

	printf("\nCiphertext\n");
	for(i = 0; i < 16; i ++)
	{
		printf("%02X", C[i]);
	}

	aes_128_decrypt(C, tempOut, RK);
	printf("\nDecrypted CT\n");
	for(i = 0; i < 16; i ++)
	{
		printf("%02X", tempOut[i]);
	}
	printf("\nTesting done\n");

	AES_KEY enc_key;
	AES_set_encrypt_key( tempKey, 128, &enc_key );
	AES_encrypt(tempMsg, C, &enc_key);
	printf("\nOpenSSL Ciphertext\n");
	for(i = 0; i < 16; i ++)
	{
		printf("%02X", C[i]);
	}


	return 0;
}

int main(int argc, char* argv[])
{
	aesTesting();

	return 0;
}
