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

gmp_randstate_t *seedRandGenFromISAAC(randctx *ctx)
{
	gmp_randstate_t *state = (gmp_randstate_t*) calloc(1, sizeof(gmp_randstate_t));
	unsigned char *seedBytes = generateIsaacRandBytes(ctx, sizeof(unsigned long int), sizeof(unsigned long int));


	unsigned long int seed = 0;
	memcpy(&seed, seedBytes, sizeof(unsigned long int));


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


void getPrimeGMP_Alt(mpz_t outputP, mpz_t outputQ, gmp_randstate_t state, int keySize)
{
	mpz_urandomb(outputP, state, keySize);
	mpz_setbit (outputP, keySize);
	mpz_nextprime(outputP, outputP);

	mpz_sub_ui(outputQ, outputP, 1);
}


// Generates a "Safe prime", that is of the form q = 2*p+1 where p and q are primes.
// An alternative way to think of it is q | (p - 1)
void getSafePrimeGMP(mpz_t outputP, mpz_t outputQ, gmp_randstate_t state, int keySize)
{
	mpz_t p, lesserP, greaterP;
	int lesserP_prime = 0;
	int greaterP_prime = 0;

	mpz_init(p);
	mpz_init(lesserP);
	mpz_init(greaterP);

	do
	{
		do
		{
			getPrimeGMP(p, state, keySize - 1);

			mpz_mul_ui(greaterP, p, 2);
			mpz_add_ui(greaterP, greaterP, 1);

			mpz_sub_ui(lesserP, p, 1);
			mpz_fdiv_q_ui(lesserP, lesserP, 2);

			lesserP_prime = mpz_probab_prime_p(lesserP, 35);
			greaterP_prime = mpz_probab_prime_p(greaterP, 35);
			fflush(stdout);
		}
		while(0 == lesserP_prime && 0 == greaterP_prime);

		// We've got two primes, now set the output to the greater of the two primes
		if(0 != greaterP_prime)
		{
			mpz_set(outputP, greaterP);
			mpz_set(outputQ, p);
		}
		else
		{
			mpz_set(outputP, p);
			mpz_set(outputQ, lesserP);
		}
	} while(128 != mpz_sizeinbase(outputQ, 2));

	printf(">>> %d\n", mpz_sizeinbase(outputQ, 2));

	gmp_printf("%Zx\n\n%Zx\n\n", outputP, outputQ);
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


unsigned char *convertMPZToBytes(mpz_t input, int *outputLength)
{
	unsigned char *bytesToOutput;

	*outputLength = sizeof(mp_limb_t) * mpz_size(input);
	bytesToOutput = (unsigned char*) calloc(*outputLength, sizeof(unsigned char));

	// *rop, *countp, order, size, endian, nails, op
	mpz_export(bytesToOutput, NULL, 1, sizeof( mp_limb_t ), 0, 0, input);

	return bytesToOutput;
}


unsigned char *convertMPZToBytesFixedLen(mpz_t input, int length)
{
	unsigned char *bytesToOutput = (unsigned char*) calloc(length, sizeof(unsigned char));
	unsigned char *tempBytes;
	int outputLength = sizeof(mp_limb_t) * mpz_size(input);

	tempBytes = (unsigned char *) calloc(outputLength, sizeof(unsigned char));
	// *rop, *countp, order, size, endian, nails, op
	mpz_export(bytesToOutput, NULL, 1, sizeof( mp_limb_t ), 0, 0, input);

	memcpy(bytesToOutput, tempBytes, outputLength);

	return bytesToOutput;
}


// We assume space has already been calloc-ed for z.
void convertBytesToMPZ(	mpz_t *z, unsigned char *input, int inputLength)
{
	mpz_init(*z);

	mpz_import(*z, inputLength / sizeof(mp_limb_t), 1, sizeof( mp_limb_t ), 0, 0, input);
}

int sizeOfSerialMPZ(mpz_t inputMPZ)
{
	int length = mpz_size(inputMPZ) * sizeof(mp_limb_t) + sizeof(int);

	return length;
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
	int tempLength;

	memcpy(&tempLength, inputBuffer + *bufferOffset, sizeof(int));
	(*bufferOffset) += sizeof(int);

	convertBytesToMPZ(outputMPZ, inputBuffer + *bufferOffset, tempLength);
	(*bufferOffset) += tempLength;

	return outputMPZ;
}



unsigned char *serialiseMPZ_Array(mpz_t *z, int arrayLen, int *bufferOffset)
{
	unsigned char *tempBuffer;
	int tempLength = sizeof(int) * (arrayLen + 1);
	int tempOffset = sizeof(int), i;

	for(i = 0; i < arrayLen; i ++)
	{
		tempLength += (mpz_size(z[i]) * sizeof(mp_limb_t));
	}

	tempBuffer = (unsigned char *) calloc(tempLength, sizeof(unsigned char));

	memcpy(tempBuffer, &arrayLen, sizeof(int));

	for(i = 0; i < arrayLen; i ++)
	{
		serialiseMPZ(z[i], tempBuffer, &tempOffset);
	}

	*bufferOffset = tempOffset;

	return tempBuffer;
}


mpz_t *deserialiseMPZ_Array(unsigned char *inputBuffer, int *bufferOffset)
{
	mpz_t *outputArray, *temp;
	int tempLength, tempOffset = *bufferOffset, i;

	memcpy(&tempLength, inputBuffer + tempOffset, sizeof(int));
	tempOffset += sizeof(int);

	outputArray = (mpz_t*) calloc(tempLength, sizeof(mpz_t));

	for(i = 0; i < tempLength; i ++)
	{
		temp = deserialiseMPZ(inputBuffer, &tempOffset);
		mpz_set(outputArray[i], *temp);
		free(temp);
	}


	*bufferOffset = tempOffset;

	return outputArray;
}


// Following code for calculating the square root of a number in a Finite field
// provided by David Gillies, all credit to him. Originall posted at
// https://gmplib.org/list-archives/gmp-discuss/2013-April/005304.html


// find x^2 = q mod n
// return
// -1 q is quadratic non-residue mod n
//  1 q is quadratic residue mod n
//  0 q is congruent to 0 mod n
//
int quadratic_residue(mpz_t x, mpz_t q, mpz_t n)
{
	int leg;
	mpz_t tmp, ofac, nr, t, r, c, b;
	unsigned int mod4;
	int twofac = 0, m, i, ix;


	mod4=mpz_tstbit(n,0);
	if(!mod4) // must be odd
		return 0;

	mod4 += 2 * mpz_tstbit(n,1);


	mpz_init_set(tmp, n);
	mpz_add_ui(tmp, tmp, 1UL);
	mpz_tdiv_q_2exp(tmp, tmp, 2);
	mpz_powm(x, q, tmp, n);
	mpz_clear(tmp);


	return 1;
}


/*
// This is the original function
int quadratic_residue_alt(mpz_t x,mpz_t q,mpz_t n)
{
    int                        leg;
    mpz_t                        tmp,ofac,nr,t,r,c,b;
    unsigned int            mod4;
    mp_bitcnt_t                twofac=0,m,i,ix;

    mod4=mpz_tstbit(n,0);
    if(!mod4) // must be odd
        return 0;

    mod4+=2*mpz_tstbit(n,1);

    leg=mpz_legendre(q,n);
    if(leg!=1)
        return leg;

    mpz_init_set(tmp,n);

    if(mod4==3) // directly, x = q^(n+1)/4 mod n
        {
        mpz_add_ui(tmp,tmp,1UL);
        mpz_tdiv_q_2exp(tmp,tmp,2);
        mpz_powm(x,q,tmp,n);
        mpz_clear(tmp);
        }
    else // Tonelli-Shanks
        {
        mpz_inits(ofac,t,r,c,b,NULL);

        // split n - 1 into odd number times power of 2 ofac*2^twofac
        mpz_sub_ui(tmp,tmp,1UL);
        twofac=mpz_scan1(tmp,twofac); // largest power of 2 divisor
        if(twofac)
            mpz_tdiv_q_2exp(ofac,tmp,twofac); // shift right

        // look for non-residue
        mpz_init_set_ui(nr,2UL);
        while(mpz_legendre(nr,n)!=-1)
            mpz_add_ui(nr,nr,1UL);

        mpz_powm(c,nr,ofac,n); // c = nr^ofac mod n

        mpz_add_ui(tmp,ofac,1UL);
        mpz_tdiv_q_2exp(tmp,tmp,1);
        mpz_powm(r,q,tmp,n); // r = q^(ofac+1)/2 mod n

        mpz_powm(t,q,ofac,n);
        mpz_mod(t,t,n); // t = q^ofac mod n

        if(mpz_cmp_ui(t,1UL)!=0) // if t = 1 mod n we're done
            {
            m=twofac;
            do
                {
                i=2;
                ix=1;
                while(ix<m)
                    {
                    // find lowest 0 < ix < m | t^2^ix = 1 mod n
                    mpz_powm_ui(tmp,t,i,n); // repeatedly square t
                    if(mpz_cmp_ui(tmp,1UL)==0)
                        break;
                    i<<=1; // i = 2, 4, 8, ...
                    ix++; // ix is log2 i
                    }
                mpz_powm_ui(b,c,1<<(m-ix-1),n); // b = c^2^(m-ix-1) mod n
                mpz_mul(r,r,b);
                mpz_mod(r,r,n); // r = r*b mod n
                mpz_mul(c,b,b);
                mpz_mod(c,c,n); // c = b^2 mod n
                mpz_mul(t,t,c);
                mpz_mod(t,t,n); // t = t b^2 mod n
                m=ix;
                }while(mpz_cmp_ui(t,1UL)!=0); // while t mod n != 1
            }
        mpz_set(x,r);
        mpz_clears(tmp,ofac,nr,t,r,c,b,NULL);
        }

    return 1;
}
*/



#endif


