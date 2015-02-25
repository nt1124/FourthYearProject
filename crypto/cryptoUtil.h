#ifndef CRYPTO_UTIL
#define CRYPTO_UTIL

#include "aes.h"
#include <string.h>
#include <time.h>

#include "cryptoUtil.c"
#include "gmpUtils.c"
#include "rsa.c"
#include "DDH_Primitive.h"
#include "EllipticCurves/eccUtils.c"

#include "OT/otToy.c"
#include "OT/otSemiHonestRSA.c"
#include "OT/otPVW_DDH.h"
#include "OT/ot_LP_CnC.h"

#include "Hashing/sha256.h"

#include "CommitmentSchemes/commit_elgamal.h"
#include "CommitmentSchemes/commit_hash_elgamal.h"
#include "ConsistencyChecks/builder_keys.h"

#include "SecretSharing/lagrangeInterpolation.h"
#include "ZeroKnowledgeProofs/ZK_PK_DDH_Tuple.h"


const int wireKeysLengths = 16;

typedef struct wireKeyPair
{
	unsigned char zeroKey[16];
	unsigned char  oneKey[16];
} wireKeyPair;

#endif