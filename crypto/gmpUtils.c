#ifndef CUSTOM_GMP_UTIL
#define CUSTOM_GMP_UTIL

#include <gmp.h>


typedef struct mpzAsHex
{
	unsigned char *payload;
	int numOfBytes;
} mpzAsHex;


gmp_randstate_t *seedRandGen()
{
    gmp_randstate_t *state = (gmp_randstate_t*) calloc(1, sizeof(gmp_randstate_t));
    unsigned long int seed = time(NULL);
    gmp_randinit_default(*state);
    gmp_randseed_ui(*state, seed);

    return state;
}


void getPrimeGMP(mpz_t output, gmp_randstate_t state, int keySize)
{
    mpz_urandomb(output, state, keySize);
    mpz_setbit (output, keySize);
    mpz_nextprime(output, output);
}


// Generates a "Safe prime", that is of the form q = 2*p+1 where p and q are primes.
void getSafePrimeGMP(mpz_t output, gmp_randstate_t state, int keySize)
{
	mpz_t p, lesserP, greaterP;
	int lesserP_prime = 0;
	int greaterP_prime = 0;

	mpz_init(p);
	mpz_init(lesserP);
	mpz_init(greaterP);

	do
	{
		getPrimeGMP(p, state, keySize-1);

		mpz_mul_ui(greaterP, p, 2);
		mpz_add_ui(greaterP, greaterP, 1);

		mpz_sub_ui(lesserP, p, 1);
		mpz_fdiv_q_ui(lesserP, lesserP, 2);

		lesserP_prime = mpz_probab_prime_p(lesserP, 25);
		greaterP_prime = mpz_probab_prime_p(greaterP, 25);
	}
	while(1 <= lesserP_prime && 1 <= greaterP_prime);

	// We've got two primes, now set the output to the greater of the two primes
	if(1 <= greaterP_prime)
		mpz_set(output, greaterP);
	else
		mpz_set(output, p);
}


int numberOfHexChars(mpz_t input, int *shift)
{
	int sizeInHex = mpz_sizeinbase(input, 16);

	if( 1 == sizeInHex % 2)
	{
		sizeInHex ++;
		*shift = 1;
	}

	return sizeInHex / 2;
}


int convertHexCharToInt(char inputChar)
{
	int returnValue = 0;

	if((int)inputChar <= 57)
	{
		returnValue = (int)inputChar - 48;
	}
	else if((int)inputChar <= 70)
	{
		returnValue = (int)inputChar - 55;
	}

	return returnValue;
}


void convertHexStringToBytes(unsigned char* outputStr, char* inputStr, int lengthOfInput)
{
	int i, lsNibble, msNibble, j = 0;
	// size_t lengthOfInput = strlen(inputStr);

	for(i = 0; i < lengthOfInput; i += 2)
	{
		// ls = Less Significant, ms = Most Significant
		lsNibble = convertHexCharToInt(inputStr[i + 1]);
		msNibble = convertHexCharToInt(inputStr[i]);

		*(outputStr + j) = 16 * msNibble + lsNibble;
		j ++;
	}
}


char *convertBytesToHex(unsigned char *input, int inputLength)
{
	const char hexConv[16] = {	'0', '1', '2', '3',
								'4', '5', '6', '7',
							  	'8', '9', 'A', 'B',
							  	'C', 'D', 'E', 'F'};
	int i, j = 0;
	unsigned char temp;
	char *output = (char*) calloc(inputLength * 2 + 1, sizeof(char));

	for(i = 0; i < inputLength; i ++)
	{
		temp = (*(input + i) & 0xf0) >> 4;
		*(output + j) = hexConv[temp];
		j ++;

		temp = *(input + i) & 0x0f;
		*(output + j) = hexConv[temp];
		j ++;
	}

	return output;
}


unsigned char *convertMPZToBytes(mpz_t input, int *inputLength)
{
	unsigned char *bytesToOutput;

	*inputLength = sizeof(mp_limb_t) * mpz_size(input);
	bytesToOutput = (unsigned char*) calloc(*inputLength, sizeof(unsigned char));

	// *rop, *countp, order, size, endian, nails, op
	mpz_export(bytesToOutput, NULL, 1, sizeof( mp_limb_t ), 0, 0, input);

	return bytesToOutput;
}


// We assume space has already been calloc-ed for z.
void convertBytesToMPZ(	mpz_t *z, unsigned char *input, int inputLength)
{
	mpz_init(*z);

	mpz_import(*z, inputLength / sizeof(mp_limb_t), 1, sizeof( mp_limb_t ), 0, 0, input);
}


void serialiseMPZ(mpz_t z, unsigned char *outputBuffer, int *bufferOffset)
{
	unsigned char *tempBuffer;
	int tempLength;

	tempBuffer = convertMPZToBytes(z, &tempLength);
	memcpy(outputBuffer + *bufferOffset, &tempLength, sizeof(int));
	(*bufferOffset) += sizeof(int);

	memcpy(outputBuffer + *bufferOffset, tempBuffer, tempLength);
	(*bufferOffset) += tempLength;
}


mpz_t *deserialiseMPZ(unsigned char *inputBuffer, int *bufferOffset)
{
	mpz_t *outputMPZ = (mpz_t*) calloc(1, sizeof(mpz_t));
	unsigned char *tempBuffer;
	int tempLength;

	memcpy(&tempLength, inputBuffer + *bufferOffset, sizeof(int));
	(*bufferOffset) += sizeof(int);

	convertBytesToMPZ(outputMPZ, inputBuffer + *bufferOffset, tempLength);
	(*bufferOffset) += tempLength;

	return outputMPZ;
}

#endif


