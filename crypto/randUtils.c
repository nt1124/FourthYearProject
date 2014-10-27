#ifndef RAND_UTILS
#define RAND_UTILS

#include <time.h>
#include <stdio.h>
#include <stdlib.h>


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

#endif