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
	mpz_t *input0 = calloc(1, sizeof(mpz_t));
	mpz_t *input1 = calloc(1, sizeof(mpz_t));
	mpz_t *enc0Num, *enc1Num;

	unsigned char *n0Bytes, *e0Bytes, *n1Bytes, *e1Bytes;
	int n0Length = 0, e0Length = 0, n1Length = 0, e1Length = 0;
	unsigned char *enc0Bytes, *enc1Bytes;
	int enc0Length = 0, enc1Length = 0;

	convertBytesToMPZ(input0, input0Bytes, inputLengths);
	convertBytesToMPZ(input1, input1Bytes, inputLengths);

	// We get PK0, PK1 from receiver here. Receive!
	printf("Reading PK0 from socket\n");
	n0Bytes = (unsigned char*) readFromSock(sockfd, &n0Length);
	e0Bytes = (unsigned char*) readFromSock(sockfd, &e0Length);
	PK0 = bytesToPubKey(n0Bytes, n0Length, e0Bytes, e0Length);

	printf("Reading PK1 from socket\n");
	n1Bytes = (unsigned char*) readFromSock(sockfd, &n1Length);
	e1Bytes = (unsigned char*) readFromSock(sockfd, &e1Length);
	PK1 = bytesToPubKey(n1Bytes, n1Length, e1Bytes, e1Length);

	printf("Compute encrypted values.\n");
	enc0Num = encRSA(*input0, PK0);
	enc1Num = encRSA(*input1, PK1);

	printf("Convert encrypted values to bytes.\n");
	enc0Bytes = convertMPZToBytes(*enc0Num, &enc0Length);
	enc1Bytes = convertMPZToBytes(*enc1Num, &enc1Length);

	/*
	// Send enc0Bytes and enc1Bytes to Receiver
	writeToSock(sockfd, (char*)enc0Bytes, enc0Length);
	writeToSock(sockfd, (char*)enc1Bytes, enc1Length);
	*/
}





unsigned char *receiverOT_SH_RSA(int sockfd, int inputBit, int *outputLength)
{
	struct rsaPubKey *PK0, *PK1;
	struct rsaPrivKey *SKi;

	mpz_t *outputNum, *tempEncNum;
	int encOutputLen;
	unsigned char *enc0Bytes, *enc1Bytes, *outputBytes;
	unsigned char *n0Bytes, *e0Bytes, *n1Bytes, *e1Bytes;
	int n0Length = 0, e0Length = 0, n1Length = 0, e1Length = 0;

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
	// pubKeyToBytes(PK0, n0Bytes, &n0Length, e0Bytes, &e0Length);
	n0Bytes = convertMPZToBytes(PK0 -> N, &n0Length);
	e0Bytes = convertMPZToBytes(PK0 -> e, &e0Length);
	writeToSock(sockfd, (char*)n0Bytes, n0Length);
	writeToSock(sockfd, (char*)e0Bytes, e0Length);

	// pubKeyToBytes(PK1, n1Bytes, &n1Length, e1Bytes, &e1Length);
	n1Bytes = convertMPZToBytes(PK1 -> N, &n1Length);
	e1Bytes = convertMPZToBytes(PK1 -> e, &e1Length);
	writeToSock(sockfd, (char*)n1Bytes, n1Length);
	writeToSock(sockfd, (char*)e1Bytes, e1Length);

	// receive enc0Bytes and enc1Bytes from Receiver
	// enc0Bytes = (unsigned char*) readFromSock(sockfd);
	// enc1Bytes = (unsigned char*) readFromSock(sockfd);

	/*
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
	*/

	return outputBytes;
}



void testSender_OT_SH_RSA(char *portNumStr)
{
    int sockfd, portNum, n;
    struct sockaddr_in serv_addr;
    
    portNum = atoi(portNumStr);
    sockfd = openSock();

    serv_addr = getServerAddr("127.0.0.1", portNum);
    connectToServer(&sockfd, serv_addr);

    unsigned char input0[16] = "1111111111111111";
    unsigned char input1[16] = "2222222222222222";


	senderOT_SH_RSA(sockfd, input0, input1, 16);
}

void testReceiver_OT_SH_RSA(char *portNumStr)
{

    int sockfd, newsockfd, clilen;
    struct sockaddr_in serv_addr, cli_addr;
    unsigned char *buffer;
    int bufferLength = 0;

    sockfd = openSock();
    serv_addr = getSelfAsServer(portNumStr);

    bindAndListenToSock(sockfd, serv_addr);
    clilen = sizeof(cli_addr);
    newsockfd = acceptNextConnectOnSock(sockfd, &cli_addr, &clilen);

    receiverOT_SH_RSA(newsockfd, 0, &bufferLength);
}



int main(int argc, char *argv[])
{
	int mode = atoi(argv[1]);

	if(0 == mode)
	{
		testSender_OT_SH_RSA(argv[2]);
	}
	else
	{
		testReceiver_OT_SH_RSA(argv[2]);
	}

	return 0;
}
