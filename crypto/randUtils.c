#ifndef RAND_UTILS
#define RAND_UTILS

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "ISAAC/rand.c"


void initRandGen()
{
	srand(time(NULL));
}


unsigned char *generateRandBytes(int randBytes, int outLen)
{
	unsigned char *outputBytes = (unsigned char*) calloc(outLen, sizeof(unsigned char));
	int i;

	for(i = 0; i < randBytes; i ++)
	{
		outputBytes[i] = (unsigned char) (rand() & 0xFF);
	}

	return outputBytes;
}


// Generates a uchar array that is outLen long where the first randBytes many are set randomly.
unsigned char *generateIsaacRandBytes(randctx *ctx, int randBytes, int outLen)
{
	unsigned char *outputBytes = (unsigned char*) calloc(outLen, sizeof(unsigned char));
	int i;

	isaac(ctx);

	for(i = 0; i < randBytes; i ++)
	{
		outputBytes[i] = (unsigned char) (isacc_rand(ctx) & 0xFF);
	}

	return outputBytes;
}


unsigned int generateRandUint()
{
	unsigned char *asBytes = generateRandBytes(4, 4);
	unsigned int asUint = 0;

	memcpy(&asUint, asBytes, sizeof(unsigned int));

	return asUint;
}


unsigned int generateRandUint(randctx *ctx)
{
	unsigned char *asBytes = generateIsaacRandBytes(ctx, 4, 4);
	unsigned int asUint = 0;

	memcpy(&asUint, asBytes, sizeof(unsigned int));

	return asUint;
}


unsigned int *generateRandUintList(int numToGen)
{
	unsigned int *toReturn = (unsigned int*) calloc(numToGen, sizeof(unsigned int));
	int i;
	
	for(i = 0; i < numToGen; i ++)
	{
		toReturn[i] = generateRandUint();
	}

	return toReturn;
}


unsigned char *generateJ_Set_ISAAC(randctx *ctx, int jSetLength)
{
	unsigned char *J_set = (unsigned char *) calloc(jSetLength, sizeof(int));
	int i = 0, tempInt;

	while(i < (jSetLength / 2) )
	{
		tempInt = generateRandUint(ctx) % jSetLength;
		if(0x00 == J_set[tempInt])
		{
			J_set[tempInt] = 0x01;
			i ++;
		}
	}

	return J_set;
}


#endif