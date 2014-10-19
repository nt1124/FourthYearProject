#ifndef CRYPTO_UTIL
#define CRYPTO_UTIL

#include "cryptoUtil.c"
#include "gmpUtils.c"
#include "elgamal.c"
#include "rsa.c"


const int wireKeysLengths = 16;

typedef struct wireKeyPair
{
	unsigned char zeroKey[16];
	unsigned char  oneKey[16];
} wireKeyPair;

#endif