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
	//encode_length( tempMsg, intMessage);

	unsigned int *RK = new uint[44];
	octet* C = new octet[16];

	for(i = 0; i < 16; i ++)
	{
		printf("%02X", tempMsg[i]);
	}
	printf("\n");

	for(i = 0; i < 16; i ++)
	{
		printf("%02X", tempKey[i]);
	}
	printf("\n");

	aes_schedule( 128, 10, tempKey, RK );
	aes_128_encrypt(C, tempMsg, RK);

	for(i = 0; i < 16; i ++)
	{
		printf("%02X", C[i]);
	}
	printf("\nTesting done\n");

	return 0;
}

int main(int argc, char* argv[])
{
	aesTesting();

	return 0;
}
