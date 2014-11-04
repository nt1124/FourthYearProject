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
    printf("Key Size = %d\n", keySize);
    mpz_setbit (output, keySize);
    mpz_nextprime(output, output);
    printf("###  -->  ###\n");
    fflush(stdout);
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


// We assume that memory for *z has already been calloc-ed
void convertBytesToMPZ(	mpz_t *z, unsigned char *input, int inputLength)
{
	mpz_init(*z);

	char *hexStr = convertBytesToHex(input, inputLength);
	mpz_set_str(*z, hexStr, 16);
	free(hexStr);
}


unsigned char *convertMPZToBytes(mpz_t input, int *inputLength)
{
	int shift = 0;
	*inputLength = numberOfHexChars(input, &shift);

	printf("inputLength --> %d\n", *inputLength);
	unsigned char *bytesToOutput = (unsigned char*) calloc( *inputLength, sizeof(unsigned char));
	char *hexVersion = (char*) calloc(*inputLength, sizeof(char));

	*(hexVersion) = '0';

	mpz_get_str( (hexVersion + shift), -16, input);

	convertHexStringToBytes(bytesToOutput, hexVersion, (*inputLength) * 2);

	return bytesToOutput;
}


unsigned char *convertMPZToBytesAlt(mpz_t input, int *inputLength)
{
	int sizeInLimbs;
	unsigned char *bytesToOutput;

	*inputLength = sizeof(mp_limb_t) * mpz_size(input);
	bytesToOutput = (unsigned char*) calloc(*inputLength, sizeof(unsigned char));

	// *rop, *countp, order, size, endian, nails, op
	mpz_export(bytesToOutput, NULL, 1, sizeof( mp_limb_t ), 0, 2, input);

	printf("inputLength ++--> %d  ==  %d\n", *inputLength, sizeInLimbs);

	return bytesToOutput;
}

// We assume space has already been calloc-ed for z.
void convertBytesToMPZAlt(	mpz_t *z, unsigned char *input, int inputLength)
{
	mpz_init(*z);

	mpz_import(*z, inputLength, 1, sizeof( mp_limb_t ), 0, 0, input);
}

#endif


