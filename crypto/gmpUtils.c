#ifndef CUSTOM_GMP_UTIL
#define CUSTOM_GMP_UTIL

#include <gmp.h>

void getPrimeGMP(mpz_t output, gmp_randstate_t state, int keySize)
{
    mpz_urandomb(output, state, keySize);
    mpz_setbit (output, keySize);
    mpz_nextprime(output, output);
}

#endif