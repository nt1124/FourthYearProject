#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../comms/sockets.h"



void senderOT_SH_RSA(int writeSocket, unsigned char *input0Bytes, unsigned char *input1Bytes, int inputLengths, mpz_t N)
{
	struct rsaPubKey *PK0, *PK1;
	mpz_t *input0 = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_t *input1 = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_t *enc0Num, *enc1Num;

	unsigned char *n0Bytes, *e0Bytes, *n1Bytes, *e1Bytes;
	unsigned char *enc0Bytes, *enc1Bytes;
	int n0Length = 0, e0Length = 0, n1Length = 0, e1Length = 0;
	int enc0Length = 0, enc1Length = 0;

	convertBytesToMPZAlt(input0, input0Bytes, inputLengths);
	convertBytesToMPZAlt(input1, input1Bytes, inputLengths);

	// We get PK0, PK1 from receiver here. Receive!
	n0Bytes = receiveBoth(writeSocket, n0Length);
	e0Bytes = receiveBoth(writeSocket, e0Length);
	PK0 = bytesToPubKey(n0Bytes, n0Length, e0Bytes, e0Length);
	// PK0 = bytesToPubKeyAlt(N, e0Bytes, e0Length);

	n1Bytes= receiveBoth(writeSocket, n1Length);
	e1Bytes= receiveBoth(writeSocket, e1Length);
	PK1 = bytesToPubKey(n1Bytes, n1Length, e1Bytes, e1Length);
	// PK1 = bytesToPubKeyAlt(N, e1Bytes, e1Length);

	enc0Num = encRSA(*input0, PK0);
	enc1Num = encRSA(*input1, PK1);

	enc0Bytes = convertMPZToBytesAlt(*enc0Num, &enc0Length);
	enc1Bytes = convertMPZToBytesAlt(*enc1Num, &enc1Length);

	// Send enc0Bytes and enc1Bytes to Receiver
	sendBoth(writeSocket, (octet*) enc0Bytes, enc0Length);
	sendBoth(writeSocket, (octet*) enc1Bytes, enc1Length);

	free(enc0Bytes); 	free(enc1Bytes);
	free(enc0Num); 		free(enc1Num);
	free(n0Bytes);		free(n1Bytes);
	free(e0Bytes);		free(e1Bytes);
	free(PK0); 			free(PK1);
}


unsigned char *receiverOT_SH_RSA(struct rsaPrivKey *SKi, gmp_randstate_t *state, int readSocket, int inputBit, int *outputLength)
{
	struct rsaPubKey *PK0, *PK1;
	unsigned char *enc0Bytes, *enc1Bytes, *outputBytes;
	unsigned char *n0Bytes, *e0Bytes, *n1Bytes, *e1Bytes;
	int n0Length = 0, e0Length = 0, n1Length = 0, e1Length = 0;
	int enc0Length, enc1Length;
	mpz_t *outputNum, *tempEncNum = (mpz_t*) calloc(1, sizeof(mpz_t));


	if(0 == inputBit)
	{
		PK0 = initPubKeyRSA();
		updateRSAKey(SKi, PK0, *state);
		PK1 = generateDudPubRSAKey(*state, SKi -> N, SKi -> thetaN);
	}
	else
	{
		PK1 = initPubKeyRSA();
		updateRSAKey(SKi, PK1, *state);
		PK0 = generateDudPubRSAKey(*state, SKi -> N, SKi -> thetaN);
	}

	// We send PK0 to sender here.
	n0Bytes = convertMPZToBytesAlt(PK0 -> N, &n0Length);
	e0Bytes = convertMPZToBytesAlt(PK0 -> e, &e0Length);
	sendBoth(readSocket, (octet*) n0Bytes, n0Length);
	sendBoth(readSocket, (octet*) e0Bytes, e0Length);

	// We send PK1 to sender here.
	n1Bytes = convertMPZToBytesAlt(PK1 -> N, &n1Length);
	e1Bytes = convertMPZToBytesAlt(PK1 -> e, &e1Length);
	sendBoth(readSocket, (octet*) n1Bytes, n1Length);
	sendBoth(readSocket, (octet*) e1Bytes, e1Length);

	// receive enc0Bytes and enc1Bytes from Receiver
	enc0Bytes = receiveBoth(readSocket, enc0Length);
	enc1Bytes = receiveBoth(readSocket, enc1Length);

	if(0 == inputBit)
		convertBytesToMPZAlt(tempEncNum, enc0Bytes, enc0Length);
	else
		convertBytesToMPZAlt(tempEncNum, enc1Bytes, enc1Length);

	free(enc0Bytes);	free(enc1Bytes);
	free(n0Bytes);		free(e0Bytes);
	free(n1Bytes);		free(e1Bytes);
	free(PK0);			free(PK1);

	outputNum = decRSA(*tempEncNum, SKi);
	outputBytes = convertMPZToBytesAlt(*outputNum, outputLength);

	free(tempEncNum);
		
	return outputBytes;
}


/*
void testSender_OT_SH_RSA(char *portNumStr)
{
	int sockfd, portNum, n;
	struct sockaddr_in serv_addr;
	char *ipAddress = (char*) calloc(10, sizeof(char));
	strncpy(ipAddress, "127.0.0.1", 9);

	portNum = atoi(portNumStr);
	sockfd = openSock();

	serv_addr = getServerAddr(ipAddress, portNum);
	connectToServer(&sockfd, serv_addr);

	unsigned char input0[17] = "1111111111111111";
	unsigned char input1[17] = "2222222222222222";

	// We could use mpz_out_raw / mpz_in_raw to send raw bytes.
	senderOT_SH_RSA(sockfd, input0, input1, 16);

	senderOT_SH_RSA(sockfd, input0, input1, 16);
}


void testReceiver_OT_SH_RSA(char *portNumStr)
{

	int sockfd, newsockfd, clilen;
	struct sockaddr_in serv_addr, cli_addr;
	unsigned char *buffer, *buffer2;
	int bufferLength = 0, i;
	struct rsaPrivKey *SKi;

	printf("Random state initialised and seeded\n");
	gmp_randstate_t *state = seedRandGen();

	sockfd = openSock();
	serv_addr = getSelfAsServer(portNumStr);

	bindAndListenToSock(sockfd, serv_addr);
	clilen = sizeof(cli_addr);
	newsockfd = acceptNextConnectOnSock(sockfd, &cli_addr, &clilen);

	SKi = generatePrivRSAKey(*state);

	buffer = receiverOT_SH_RSA(SKi, state, newsockfd, 0, &bufferLength);
	printf("Requesting 0 gave us...\n");
	for(i = 0; i < bufferLength; i ++)
	{
		printf("%02X", buffer[i]);
	} printf("\n");
	free(buffer);
	printf("\nDone with round one.\n");

	buffer2 = receiverOT_SH_RSA(SKi, state, newsockfd, 1, &bufferLength);
	printf("Requesting 1 gave us...\n");
	for(i = 0; i < bufferLength; i ++)
	{
		printf("%02X", buffer2[i]);
	} printf("\n");
	free(buffer2);
}
*/

int testEncDec(struct rsaPubKey *PK, struct rsaPrivKey *SK)
{
	unsigned char input0[17] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
								0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	mpz_t *inputInt = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_t *encInt = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_t *decInt = (mpz_t*) calloc(1, sizeof(mpz_t));
	unsigned char *outputBytes1, *outputBytes2;
	int tempInt1, tempInt2;

	strncpy((char*)input0, "1111111111111111", 16);

	convertBytesToMPZ(inputInt, input0, 16);
	gmp_printf("%Zd\n", *inputInt);
	encInt = encRSA(*inputInt, PK);
	decInt = decRSA(*encInt, SK);

	outputBytes2 = convertMPZToBytes(*inputInt, &tempInt2);
	outputBytes1 = convertMPZToBytes(*decInt, &tempInt1);

	if(16 == tempInt1)
		if(strncmp((char*)outputBytes1, (char*)input0, tempInt1) == 0)
			return 1;

	int i;
	printf("%d = %d\n", 16, tempInt1);
	for(i = 0; i < tempInt1; i ++)
	{    	
		printf("%u + %u + %u\n", input0[i], outputBytes1[i], outputBytes2[i]);
	}

	return 0;
}