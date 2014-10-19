/*
virtual void 	preCompute() = 0;
virtual int  	send(unsigned char *buffer, int bufferSize) = 0;
virtual int 	listen(unsigned char *buffer, int bufferSize) = 0;
virtual int 	transfer() = 0;
*/

#include <openssl/rsa.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


const int padding = RSA_PKCS1_PADDING;


void send()
{

}


int main()
{
	RSA *r;
	unsigned char message[2048/8] = "123456789012345678901234567890\0";
	unsigned char *encrypted = (unsigned char*) calloc(2048/8, sizeof(unsigned char));

	generate_key(r);
	// public_encrypt(message, strlen(message), r, encrypted);
	private_decrypt(message, strlen(message), r, encrypted);

	return 0;
}
