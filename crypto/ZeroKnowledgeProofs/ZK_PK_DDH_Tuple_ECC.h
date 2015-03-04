#ifndef ZK_PK_DDH_TUPLE_ECC
#define ZK_PK_DDH_TUPLE_ECC


typedef struct alphaAndA_Struct
{
	struct eccPoint *alpha;
	mpz_t a;
} alphaAndA_Struct;


typedef struct verifierCommitment_ECC
{
	mpz_t c;
	mpz_t t;
	struct eccPoint *C_commit;
} verifierCommitment_ECC;


typedef struct msgOneArrays_ECC
{
	struct msgOne_notI_Arrays *notI_Struct;

	int *in_I_index;
	mpz_t *roeArray;

	struct eccPoint **A_array;
	struct eccPoint **B_array;
} msgOneArrays_ECC;



#include "ZK_PK_DDH_Tuple_utils_ECC.c"
#include "ZK_PK_DDH_Tuple_ECC.c"



#endif