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


long int* convertAngledToIntArray(angledRep input)
{
	long int* toReturn = calloc(4, sizeof(long int));

	*(toReturn)		= input -> index;
	*(toReturn + 1)	= input -> delta;
	*(toReturn + 2) = input -> alpha_i;
	*(toReturn + 3) = input -> gamma_alpha_i;

	return toReturn;
}


long int* convertSquareToIntArray(squareRep input, long int numParties)
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


struct angledRep addAngledRep(struct angledRep inputA, struct angledRep inputB)
{
	struct angledRep output;

	output -> delta = inputA -> delta + inputB -> delta;
	output -> alpha_i = inputA -> alpha_i + inputB -> alpha_i;
	output -> gamma_alpha_i = inputA -> gamma_alpha_i + inputB -> gamma_alpha_i;

	return output;
}


void addConstantToAngledRep(long int constInput, struct angledRep angledInput)
{
	angledInput -> delta 	= angledInput -> delta - constInput;
	angledInput -> alpha_i 	= angledInput -> alpha_i + constInput;
}


#endif