/*
virtual void 	preCompute() = 0;
virtual int  	send(unsigned char *buffer, int bufferSize) = 0;
virtual int 	listen(unsigned char *buffer, int bufferSize) = 0;
virtual int 	transfer() = 0;
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../comms/socketUtils.c"



void senderOT_SH_RSA(int sockfd, unsigned char *input0Bytes, unsigned char *input1Bytes, int inputLengths)
{
	struct rsaPubKey *PK0, *PK1;
	mpz_t *input0 = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_t *input1 = (mpz_t*) calloc(1, sizeof(mpz_t));
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

	// Send enc0Bytes and enc1Bytes to Receiver
	writeToSock(sockfd, (char*)enc0Bytes, enc0Length);
	writeToSock(sockfd, (char*)enc1Bytes, enc1Length);

	free(enc0Bytes); 	free(enc1Bytes);
	free(enc0Num); 	free(enc1Num);
	free(n0Bytes);		free(e0Bytes);
	free(n1Bytes);		free(e1Bytes);
	free(PK0); free(PK1);
}


unsigned char *receiverOT_SH_RSA(gmp_randstate_t *state, int sockfd, int inputBit, int *outputLength)
{
	struct rsaPubKey *PK0, *PK1;
	struct rsaPrivKey *SKi;

	mpz_t *outputNum, *tempEncNum = (mpz_t*) calloc(1, sizeof(mpz_t));
	int enc0Length, enc1Length;
	unsigned char *enc0Bytes, *enc1Bytes, *outputBytes;
	unsigned char *n0Bytes, *e0Bytes, *n1Bytes, *e1Bytes;
	int n0Length = 0, e0Length = 0, n1Length = 0, e1Length = 0;

	printf("Generate SKi\n");
	SKi = generatePrivRSAKey(*state);

	printf("SKi generated. Now to generate public keys\n");
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
	printf("Sending PK0.\n");
	n0Bytes = convertMPZToBytes(PK0 -> N, &n0Length);
	e0Bytes = convertMPZToBytes(PK0 -> e, &e0Length);
	writeToSock(sockfd, (char*)n0Bytes, n0Length);
	writeToSock(sockfd, (char*)e0Bytes, e0Length);

	printf("Sending PK1.\n");
	n1Bytes = convertMPZToBytes(PK1 -> N, &n1Length);
	e1Bytes = convertMPZToBytes(PK1 -> e, &e1Length);
	writeToSock(sockfd, (char*)n1Bytes, n1Length);
	writeToSock(sockfd, (char*)e1Bytes, e1Length);

	// receive enc0Bytes and enc1Bytes from Receiver
	printf("Reading encrypted bytes.\n");
	enc0Bytes = (unsigned char*) readFromSock(sockfd, &enc0Length);
	enc1Bytes = (unsigned char*) readFromSock(sockfd, &enc1Length);

	if(0 == inputBit)
	{
		convertBytesToMPZ(tempEncNum, enc0Bytes, enc0Length);
	}
	else
	{
		convertBytesToMPZ(tempEncNum, enc1Bytes, enc1Length);
	}

	printf("Converted valid bytes to MPZ\n");
	free(enc0Bytes); 	free(enc1Bytes);
	free(n0Bytes);		free(e0Bytes);
	free(n1Bytes);		free(e1Bytes);
	free(PK0); free(PK1); free(SKi);

	printf("Decrypting answer\n");
	outputNum = decRSA(*tempEncNum, SKi);
	outputBytes = convertMPZToBytes(*outputNum, outputLength);
	printf("Successful decryption!\n");

	free(tempEncNum);

	return outputBytes;
}



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

	printf("Random state initialised and seeded\n");
    gmp_randstate_t *state = seedRandGen();

    sockfd = openSock();
    serv_addr = getSelfAsServer(portNumStr);

    bindAndListenToSock(sockfd, serv_addr);
    clilen = sizeof(cli_addr);
    newsockfd = acceptNextConnectOnSock(sockfd, &cli_addr, &clilen);


    buffer = receiverOT_SH_RSA(state, newsockfd, 0, &bufferLength);
    printf("Requesting 0 gave us...\n");
    for(i = 0; i < bufferLength; i ++)
    {
    	printf("%02X", buffer[i]);
    } printf("\n");
    free(buffer);
    printf("Done with round one.\n");

    buffer2 = receiverOT_SH_RSA(state, newsockfd, 1, &bufferLength);
    printf("Requesting 1 gave us...\n");
    for(i = 0; i < bufferLength; i ++)
    {
    	printf("%02X", buffer2[i]);
    } printf("\n");
    free(buffer2);
}



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
