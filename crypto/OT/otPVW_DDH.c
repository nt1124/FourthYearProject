#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../comms/sockets.h"
#include "../DDH_Primitive.h"


struct CRS
{
	mpz_t g_0;
	mpz_t g_1;
	mpz_t h_0;
	mpz_t h_1;
} CRS;


/*
void precompute(struct DDH_Group *group, mpz_t g0, mpz_t g1, mpz_t h0, mpz_t h1, gmp_randstate_t *state)
{
	group = (struct DDH_Group*) calloc(1, sizeof(struct DDH_Group));

	mpz_init(group -> p);
	mpz_init(group -> g);

	getPrimeGMP(group -> p, state, 1024);
	mpz_urandomm(group -> g, state, group -> p);

	CRS -> g_0 = g0;
	CRS -> g_1 = g1;
	CRS -> h_0 = h0;
	CRS -> h_1 = h1;
}
*/

void setupMessy(int securityParam, struct DDH_Group *group,
				gmp_randstate_t state)
{
	struct CRS *crs = (struct CRS*) calloc(1, sizeof(struct CRS));
	mpz_t x, y;

	group = generateGroup(securityParam, state);
	mpz_init(x);
	mpz_init(y);

	do
	{
		mpz_urandomm(crs -> g_0, state, group -> p);
	} while( 0 < mpz_cmp_ui(crs -> g_0, 1) );

	do
	{
		mpz_urandomm(crs -> g_1, state, group -> p);
	} while( 0 < mpz_cmp_ui(crs -> g_1, 1) );
	
	do
	{
		mpz_urandomm(x, state, group -> p);
	} while( 0 < mpz_cmp_ui(x, 1) );

	do
	{
		mpz_urandomm(y, state, group -> p);
	} while( 0 < mpz_cmp_ui(y, 1) );


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
