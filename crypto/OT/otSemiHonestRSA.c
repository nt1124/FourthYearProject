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
#include "../../comms/socketUtils.c"




void senderOT_SH_RSA(int sockfd, unsigned char *input0Bytes, unsigned char *input1Bytes, int inputLengths)
{
	struct rsaPubKey *PK0, *PK1;
	mpz_t *input0, *input1;
	mpz_t *enc0Num, *enc1Num;

	unsigned char *n0Bytes, *e0Bytes, *n1Bytes, *e1Bytes;
	unsigned char *enc0Bytes, *enc1Bytes;
	int enc0Length, enc1Length;


	convertBytesToMPZ(input0, input0Bytes, inputLengths);
	convertBytesToMPZ(input1, input1Bytes, inputLengths);

	// We get PK0, PK1 from receiver here. Receive!
	n0Bytes = (unsigned char*) readFromSock(sockfd);
	e0Bytes = (unsigned char*) readFromSock(sockfd);
	n1Bytes = (unsigned char*) readFromSock(sockfd);
	e1Bytes = (unsigned char*) readFromSock(sockfd);

	PK0 = bytesToPubKey(n0Bytes, e0Bytes);
	PK1 = bytesToPubKey(n1Bytes, e1Bytes);

	enc0Num = encRSA(*input0, PK0);
	enc1Num = encRSA(*input1, PK1);

	enc0Bytes = convertMPZToBytes(*enc0Num, &enc0Length);
	enc1Bytes = convertMPZToBytes(*enc1Num, &enc1Length);

	// Send enc0Bytes and enc1Bytes to Receiver
	writeToSock(sockfd, (char*)enc0Bytes, enc0Length);
	writeToSock(sockfd, (char*)enc1Bytes, enc1Length);
}





unsigned char *receiverOT_SH_RSA(int sockfd, int inputBit, int *outputLength)
{
	struct rsaPubKey *PK0, *PK1;
	struct rsaPrivKey *SKi;

	mpz_t *outputNum, *tempEncNum;
	int encOutputLen;
	unsigned char *enc0Bytes, *enc1Bytes, *outputBytes;
	unsigned char *n0Bytes, *e0Bytes, *n1Bytes, *e1Bytes;
	int n0Length, e0Length, n1Length, e1Length;

    gmp_randstate_t *state = seedRandGen();

	SKi = generatePrivRSAKey(*state);

	if(0 == inputBit)
	{
		PK0 = generatePubRSAKey(SKi);
		PK1 = generateDudPubRSAKey(*state);
	}
	else
	{
		PK1 = generatePubRSAKey(SKi);
		PK0 = generateDudPubRSAKey(*state);
	}

	// We send PK0, PK1 to sender here.
	pubKeyToBytes(PK0, n0Bytes, &n0Length, e0Bytes, &e0Length);
	writeToSock(sockfd, (char*)n0Bytes, n0Length);
	writeToSock(sockfd, (char*)e0Bytes, e0Length);

	pubKeyToBytes(PK1, n1Bytes, &n1Length, e1Bytes, &e1Length);
	writeToSock(sockfd, (char*)n1Bytes, n1Length);
	writeToSock(sockfd, (char*)e1Bytes, e1Length);

	// receive enc0Bytes and enc1Bytes from Receiver
	enc0Bytes = (unsigned char*) readFromSock(sockfd);
	enc1Bytes = (unsigned char*) readFromSock(sockfd);

	if(0 == inputBit)
	{
		encOutputLen = strlen(enc0Bytes);
		convertBytesToMPZ(tempEncNum, enc0Bytes, encOutputLen);
	}
	else
	{
		encOutputLen = strlen(enc1Bytes);
		convertBytesToMPZ(tempEncNum, enc1Bytes, encOutputLen);
	}

	outputNum = decRSA(*tempEncNum, SKi);
	convertMPZToBytes(*outputNum, outputLength);

	return outputBytes;
}


int main()
{

	return 0;
}
