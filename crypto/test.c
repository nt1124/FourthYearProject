#include "aes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int aesTesting()
{
	printf( "CPU capable?  %d\n", Check_CPU_support_AES() );

	int i;

	octet* tempKey = new octet[16];
	octet* tempMsg = new octet[16];
	octet* tempOut = new octet[16];

	initRandGen();

	printf("0 indicates succesful test.\n");
	for(i = 0; i < 100; i ++)
	{
		tempMsg = generateRandBytes(16);
		tempKey = generateRandBytes(16);

		uint *encRK = getUintKeySchedule(tempKey);
		octet* C = new octet[16];
		aes_128_encrypt(C, tempMsg, encRK);
		uint *decRK = decryptionKeySchedule_128(encRK);
		aes_128_decrypt(C, tempOut, decRK);
		
		printf("%d Test = %d\n", i, strncmp( (char*)tempMsg, (char*)tempOut, 16));
	}

	return 0;
}

int main(int argc, char* argv[])
{
	aesTesting();

	return 0;
}
