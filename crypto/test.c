#include "aes.h"
#include <stdio.h>
#include <stdlib.h>


int aesTesting()
{
	printf( "CPU capable?  %d\n", Check_CPU_support_AES() );

	int intKey = 2324253, intMessage = 123546236, i;

	octet* tempKey = new octet[4];
	tempKey[0] = (intKey >> 24) & 0x000000FF;
	tempKey[1] = (intKey >> 16) & 0x000000FF;
	tempKey[2] = (intKey >> 8) & 0x000000FF;
	tempKey[3] = (intKey >> 0) & 0x000000FF;

	octet* tempMsg = new octet[16];
	octet* tempOut = new octet[16];
	//encode_length( tempMsg, intMessage);
	tempMsg = generateRandBytes(16);

	unsigned int *RK = new uint[44];
	octet* C = new octet[16];

	printf("Message\n");
	for(i = 0; i < 16; i ++)
	{
		printf("%02X", tempMsg[i]);
	}

	printf("\nKey\n");
	for(i = 0; i < 16; i ++)
	{
		printf("%02X", tempKey[i]);
	}

	aes_schedule( 128, 10, tempKey, RK );

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

	return 0;
}

int main(int argc, char* argv[])
{
	aesTesting();

	return 0;
}
