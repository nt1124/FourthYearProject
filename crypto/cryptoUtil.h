#ifndef CRYPTO_UTIL
#define CRYPTO_UTIL

#include "aes.h"
#include <string.h>
#include <time.h>

#include "cryptoUtil.c"
#include "gmpUtils.c"
#include "rsa.c"
#include "DDH_Primitive.h"

#include "OT/otToy.c"
#include "OT/otSemiHonestRSA.c"
#include "OT/otPVW_DDH.h"
#include "OT/ot_LP_CnC.h"

#include "Hashing/sha256.h"

#include "CommitmentSchemes/commit_elgamal.h"
#include "CommitmentSchemes/commit_hash_elgamal.h"



const int wireKeysLengths = 16;

typedef struct wireKeyPair
{
	unsigned char zeroKey[16];
	unsigned char  oneKey[16];
} wireKeyPair;

#endif