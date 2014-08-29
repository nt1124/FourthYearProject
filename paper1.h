#ifndef PAPER_1
#define PAPER_1

#include <stdlib.h>
#include <stdio.h>
#include "mpiViaOMP.h"


typedef struct angledRep
{
	long int index;
	long int delta;
	long int alpha_i;
	long int gamma_alpha_i;
} angledRep;


typedef struct squareRep
{
	long int index;
	long int alpha_i;
	long int beta_i;
	long int* gamma_alpha_i;
} squareRep;


union customReps
{
	struct angledRep a;
	struct squareRep b;
} customReps;



long int* convertAngledToIntArray(struct angledRep input);
long int* convertSquareToIntArray(struct squareRep input, long int numParties);
struct angledRep convertIntArrayToAngled(long int* inputInts);
struct squareRep convertIntArrayToSquare(long int* inputInts, long int numParties);
long int gammaMAC(long int input);

void addAngledRep(struct angledRep output, struct angledRep inputA, struct angledRep inputB);
void subAngledRep(struct angledRep output, struct angledRep inputA, struct angledRep inputB);
void addConstantToAngledRep(long int constInput, struct angledRep angledInput, const int partyAltered);
void multiplyAngledRep( struct angledRep output, struct angledRep inputX, struct angledRep inputY,
						struct angledRep a, 	 struct angledRep b, 	  struct angledRep c);
void multiplyAngledByConst(struct angledRep output, long int constInput, struct angledRep angledInput);

struct angledRep* gatherAngled(struct angledRep ownShare, long int numParties, long int* commChannels);
long int* openAngled(struct angledRep ownShare, long int numParties, long int* commChannels);
long int* commChannels;


void init_comms_etc( int numParties )
{
	commChannels = calloc(sizeof(long int), numParties * numParties);
	initRandom();
}


long int* convertAngledToIntArray( struct angledRep input )
{
	long int* toReturn = (long int*) calloc( 4, sizeof(long int) );

	*(toReturn)		= input.index;
	*(toReturn + 1)	= input.delta;
	*(toReturn + 2) = input.alpha_i;
	*(toReturn + 3) = input.gamma_alpha_i;

	return toReturn;
}


long int* convertSquareToIntArray( struct squareRep input, long int numParties )
{
	long int* toReturn = (long int*) calloc(2 + numParties, sizeof(long int));
	int i;

	*(toReturn)		= input.index;
	*(toReturn + 1)	= input.alpha_i;
	*(toReturn + 2) = input.beta_i;
	for(i = 0; i < numParties - 1; i ++)
	{
		*(toReturn + 3 + i) = *(input.gamma_alpha_i + i);
	}

	return toReturn;
}

struct angledRep convertIntArrayToAngled( long int* inputInts )
{
	struct angledRep output;

	output.index = *(inputInts);
	output.delta = *(inputInts + 1);
	output.alpha_i = *(inputInts + 2);
	output.gamma_alpha_i = *(inputInts + 3);

	return output;
}


struct squareRep convertIntArrayToSquare( long int* inputInts, long int numParties )
{
	struct squareRep output;
	int i;

	output.index 	= *(inputInts);
	output.alpha_i 	= *(inputInts + 1);
	output.beta_i 	= *(inputInts + 2);

	for(i = 0; i < numParties; i ++)
	{
		*(output.gamma_alpha_i + i) = *(inputInts + 3 + i);
	}

	return output;
}


struct angledRep generateRandomAngled( long int ownIndex, long int delta )
{
	long int* rawRandom = calloc(sizeof(long int), 4);
	*(rawRandom + 0) = ownIndex;
	*(rawRandom + 1) = delta;
	*(rawRandom + 2) = getLongRandom();
	*(rawRandom + 3) = gammaMAC( *(rawRandom + 2) );

	struct angledRep toReturn = convertIntArrayToAngled(rawRandom);

	return toReturn;
}


void addAngledRep( struct angledRep output, struct angledRep inputA, struct angledRep inputB )
{
	output.index = inputA.index;
	output.delta = inputA.delta + inputB.delta;
	output.alpha_i = inputA.alpha_i + inputB.alpha_i;
	output.gamma_alpha_i = inputA.gamma_alpha_i + inputB.gamma_alpha_i;
}


void subAngledRep( struct angledRep output, struct angledRep inputA, struct angledRep inputB )
{
	output.index = inputA.index;
	output.delta = inputA.delta - inputB.delta;
	output.alpha_i = inputA.alpha_i - inputB.alpha_i;
	output.gamma_alpha_i = inputA.gamma_alpha_i - inputB.gamma_alpha_i;
}


void addConstantToAngledRep( long int constInput, struct angledRep angledInput, const int partyAltered )
{
	angledInput.delta = angledInput.delta - constInput;
	if(partyAltered == angledInput.index)
	{
		angledInput.alpha_i = angledInput.alpha_i + constInput;
	}
}


void multiplyAngledRep( struct angledRep output, struct angledRep inputX, struct angledRep inputY,
						struct angledRep a, 	 struct angledRep b, 	  struct angledRep c )
{
	//We need to open <x> - <a> to get epsilon and <y> - <b> to get delta 
	long int epsilon, delta;
	struct angledRep temp;

	multiplyAngledByConst(temp, epsilon, b);
	addAngledRep(output, temp, c);

	multiplyAngledByConst(temp, delta, a);
	addAngledRep(output, temp, output);

	addConstantToAngledRep(epsilon*delta, output, 1);
}


void multiplyAngledByConst( struct angledRep output, long int constInput, struct angledRep angledInput )
{
	output.index = angledInput.index;

	output.delta = angledInput.delta / constInput;
	output.alpha_i = angledInput.alpha_i * constInput;
	output.gamma_alpha_i = angledInput.gamma_alpha_i;
}


long int gammaMAC( long int input )
{
	return input;
}


long int* openAngled( struct angledRep ownShare, long int numParties, long int* commChannels )
{
	struct angledRep* allSharesAngled = gatherAngled(ownShare, numParties, commChannels);

	long int* sumAlphas = calloc(sizeof(long int), 1);
	long int sumGammaAlphas = 0;
	int i;

	for(i = 0; i < numParties; i ++)
	{
		*(sumAlphas) += allSharesAngled[i].alpha_i;
		sumGammaAlphas += allSharesAngled[i].gamma_alpha_i;
	}

	if( sumGammaAlphas == gammaMAC(*(sumAlphas)) )
	{
		return sumAlphas;
	}
	else
	{
		return NULL;
	}
}


struct angledRep* gatherAngled( struct angledRep ownShare, long int numParties, long int* commChannels )
{
	long int* ownShareInt = convertAngledToIntArray( ownShare );
	long int* anothersShareInt;
	struct angledRep* allSharesAngled = calloc(sizeof(struct angledRep), numParties);
	int i;

	for( i = 0; i < numParties; i ++)
	{
		if(i == ownShare.index)
		{
			blockingSameBroadcast(commChannels, ownShareInt, 4);
			*(allSharesAngled + i) = ownShare;
		}
		else
		{
			anothersShareInt = blockingMultiRecv(i, commChannels);
			*(allSharesAngled + i) = convertIntArrayToAngled(anothersShareInt);
			free(anothersShareInt);
		}
	}

	return allSharesAngled;
}


void generateMultiplicationTriple( long int numParties )
{
	long int a = getLongRandom(), b = getLongRandom();
	long int c = a * b;


}


#endif




