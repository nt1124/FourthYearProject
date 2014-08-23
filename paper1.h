#ifndef PAPER_1
#define PAPER_1



typedef struct angledRep
{
	int index;
	long int delta;
	long int alpha_i;
	long int gamma_alpha_i;
} angledRep;


typedef struct squareRep
{
	int index;
	long int alpha_i;
	long int beta_i;
	long int* gamma_alpha_i;
} squareRep;


union customReps
{
	struct angledRep a;
	struct squareRep b;
} customReps;


long int* convertAngledToInt(angledRep input)
{
	long int* toReturn = calloc(4, sizeof(long int));

	*(toReturn)		= input -> index;
	*(toReturn + 1)	= input -> delta;
	*(toReturn + 2) = input -> alpha_i;
	*(toReturn + 3) = input -> gamma_alpha_i;

	return toReturn;
}


long int* convertSquareToInt(squareRep input, long int numParties)
{
	long int* toReturn = calloc(2 + numParties, sizeof(long int));
	int i;

	*(toReturn)		= input -> index;
	*(toReturn + 1)	= input -> alpha_i;
	*(toReturn + 2) = input -> beta_i;

	for(i = 0; i < numParties - 1; i ++)
	{
		*(toReturn + 3 + i) = input -> *(gamma_alpha_i + i);
	}

	return toReturn;
}


#endif