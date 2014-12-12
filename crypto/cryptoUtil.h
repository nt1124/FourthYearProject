#ifndef CRYPTO_UTIL
#define CRYPTO_UTIL

#include "aes.h"
#include <string.h>
#include <time.h>

#include "cryptoUtil.c"
#include "gmpUtils.c"
#include "rsa.c"

#include "OT/otToy.c"
#include "OT/otSemiHonestRSA.c"
#include "OT/otPVW_DDH.h"



const int wireKeysLengths = 16;

typedef struct wireKeyPair
{
	unsigned char zeroKey[16];
	unsigned char  oneKey[16];
} wireKeyPair;

#endif