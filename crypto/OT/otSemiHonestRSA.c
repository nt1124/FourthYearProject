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


// const int padding = RSA_PKCS1_PADDING;




void send(unsigned char *input0Bytes, unsigned char *input1Bytes, int inputLengths)
{
	struct rsaPubKey *PK0, *PK1;
	mpz_t *input0, *input1;
	mpz_t *enc0Num, *enc1Num;


	convertBytesToMPZ(input0, input0Bytes, inputLengths);
	convertBytesToMPZ(input1, input1Bytes, inputLengths);

	enc0Num = encRSA(*input0, PK0);
	enc1Num = encRSA(*input1, PK1);

	// We get PK0, PK1 from receiver here.

}


int main()
{

}
