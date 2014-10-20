/*
virtual void 	preCompute() = 0;
virtual int  	send(unsigned char *buffer, int bufferSize) = 0;
virtual int 	listen(unsigned char *buffer, int bufferSize) = 0;
virtual int 	transfer() = 0;
*/

// #include <openssl/rsa.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../rsa.c"




void send(unsigned char *input0Bytes, unsigned char *input1Bytes, int inputLengths)
{
	struct rsaPubKey *PK0, *PK1;
	mpz_t *input0, *input1;
	mpz_t *enc0Num, *enc1Num;
	int enc0Length, enc1Length;
	unsigned char *enc0Bytes, *enc1Bytes;


	convertBytesToMPZ(input0, input0Bytes, inputLengths);
	convertBytesToMPZ(input1, input1Bytes, inputLengths);

	// We get PK0, PK1 from receiver here. Receive!

	enc0Num = encRSA(*input0, PK0);
	enc1Num = encRSA(*input1, PK1);

	enc0Bytes = convertMPZToBytes(*enc0Num, &enc0Length);
	enc1Bytes = convertMPZToBytes(*enc1Num, &enc1Length);

	// Send enc0Bytes and enc1Bytes to Receiver

}


int main()
{

}
