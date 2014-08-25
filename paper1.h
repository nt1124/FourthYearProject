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


struct angledRep convertIntArrayToAngled(long int inputInts)
{
	struct angledRep output;

	output -> index 		= *(inputInts);
	output -> delta 		= *(inputInts + 1);
	output -> alpha_i 		= *(inputInts + 2);
	output -> gamma_alpha_i = *(inputInts + 3);

	return output;
}


struct squareRep convertIntArrayToSquare(long int inputInts)
{
	struct squareRep output;
	int i;

	output -> index 		= *(inputInts);
	output -> alpha_i 		= *(inputInts + 1);
	output -> beta_i 		= *(inputInts + 2);
	for(i = 0; i < numParties; i ++)
	{
		output -> *(gamma_alpha_i + i) = *(inputInts + 3 + i);
	}

	return output;
}


void addAngledRep(struct angledRep output, struct angledRep inputA, struct angledRep inputB)
{
	output -> index = inputA -> index;
	output -> delta = inputA -> delta + inputB -> delta;
	output -> alpha_i = inputA -> alpha_i + inputB -> alpha_i;
	output -> gamma_alpha_i = inputA -> gamma_alpha_i + inputB -> gamma_alpha_i;
}


void subAngledRep(struct angledRep output, struct angledRep inputA, struct angledRep inputB)
{
	output -> index = inputA -> index;
	output -> delta = inputA -> delta - inputB -> delta;
	output -> alpha_i = inputA -> alpha_i - inputB -> alpha_i;
	output -> gamma_alpha_i = inputA -> gamma_alpha_i - inputB -> gamma_alpha_i;
}


void addConstantToAngledRep(long int constInput, struct angledRep angledInput, const int partyAltered)
{
	angledInput -> delta = angledInput -> delta - constInput;
	if(partyAltered == angledInput -> index)
	{
		angledInput -> alpha_i = angledInput -> alpha_i + constInput;
	}

	return angledInput;
}


void multiplyAngledRep( struct angledRep output, struct angledRep inputX, struct angledRep inputY,
						struct angledRep a, 	 struct angledRep b, 	  struct angledRep c)
{
	//We need to open <x> - <a> to get epsilon and <y> - <b> to get delta 
	long int epsilon, delta;
	struct angledRep temp;

	multiplyAngledByConst(temp, epsilon, b);
	addAngledRep(output, temp, c);

	multiplyAngledByConst(temp, delta, a);
	addAngledRep(output, temp, output);

	addConstantToAngledRep(output, epsilon*delta, output);
}


void multiplyAngledByConst(struct angledRep output, long int constInput, struct angledRep angledInput)
{
	output -> index = angledInput -> input;

	output -> delta = angledInput -> delta / constInput;
	output -> alpha_i = angledInput -> alpha_i * constInput;
	output -> gamma_alpha_i = angledInput -> gamma_alpha_i;
}


long int gammaMAC(long int input)
{
	return input;
}



#endif




