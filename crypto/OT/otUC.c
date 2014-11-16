#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../comms/sockets.h"


struct CRS
{
	mpz_t g0;
	mpz_t g1;
	mpz_t h0;
	mpz_t h1;
} CRS;

typedef struct DlogGroup
{
	mpz_t gen;
	mpz_t groupOrder;
} DlogGroup;


void precompute(struct DlogGroup *dlog, mpz_t g0, mpz_t g1, mpz_t h0, mpz_t h1, gmp_randstate_t *state)
{
	dlog = (struct DlogGroup) calloc(1, sizeof(struct DlogGroup));

	mpz_init(dlog -> groupOrder);
	mpz_init(dlog -> gen);

	getPrimeGMP(dlog -> groupOrder, state, 1024);
	mpz_urandomm(dlog -> gen, state, dlog -> groupOrder);

	CRS -> g0 = g0;
	CRS -> g1 = g1;
	CRS -> h0 = h0;
	CRS -> h1 = h1;
}


/*  SENDER
 *  Runs the transfer phase of the OT protocol
 *  ------------------------------------------
 *	Transfer Phase (with inputs x0, x1)
 *	WAIT for message from R
 *	DENOTE the values received by (g,h) 
 *	COMPUTE (u0,v0) = RAND(g0, g, h0, h)
 *	COMPUTE (u1,v1) = RAND(g1, g, h1, h)
 *	COMPUTE c0 = x0 * v0
 *	COMPUTE c1 = x1 * v1
 *	SEND (u0, c0) and (u1, c1) to R
 *	OUTPUT nothing
*/

void senderOT_UC(int writeSocket, unsigned char *input0Bytes, unsigned char *input1Bytes, int inputLengths)
{

}


/*  RECEIVER
 *  Runs the transfer phase of the OT protocol
 *  ------------------------------------------
 *  Transfer Phase (with input sigma) 
 * 	SAMPLE a random value r <- {0, . . . , q-1} 
 * 	COMPUTE
 * 	4.	g = (gSigma) ^ r
 * 	5.	h = (hSigma) ^ r
 * 	SEND (g, h) to S
 * 	WAIT for messages (u0,c0) and (u1,c1) from S
 * 	IF  NOT	(u0, u1, c0, c1) in G
 * 		REPORT ERROR
 * 	OUTPUT  xSigma = cSigma * (uSigma)^(-r)
*/

unsigned char *receiverOT_UC(gmp_randstate_t *state, int readSocket, int inputBit, int *outputLength)
{

}
