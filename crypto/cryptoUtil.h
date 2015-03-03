#ifndef CRYPTO_UTIL
#define CRYPTO_UTIL

#include "aes.h"
#include <string.h>
#include <time.h>

#include "cryptoUtil.c"
#include "gmpUtils.c"
#include "rsa.c"
#include "DDH_Primitive.h"
#include "EllipticCurves/ecc.h"
#include "EllipticCurves/ecc_Hom.h"

#include "OT/otToy.c"
#include "OT/otPVW/otPVW_DDH.h"
#include "OT/otPVW_ECC/otPVW_ECC.h"
#include "OT/otLP_CnC/ot_LP_CnC.h"
#include "OT/otLP_ECC_CnC/ot_LP_ECC_CnC.h"

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