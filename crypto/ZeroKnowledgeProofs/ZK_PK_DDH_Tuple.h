#ifndef ZK_PK_DDH_TUPLE
#define ZK_PK_DDH_TUPLE



typedef struct witnessStruct
{
	int *IDs;
	mpz_t *witnesses;
} witnessStruct;


typedef struct verifierCommitment
{
	mpz_t c;
	mpz_t t;
	mpz_t C_commit;
} verifierCommitment;


typedef struct msgOne_I_Arrays
{
	int *in_I_index;
	mpz_t *C_array;
	mpz_t *Z_array;
} msgOne_I_Arrays;


typedef struct msgOneArrays
{
	struct msgOne_I_Arrays *in_I_Struct;

	int *not_in_I_index;
	mpz_t *roeArray;

	mpz_t *A_array;
	mpz_t *B_array;
} msgOneArrays;



#include "ZK_PK_DDH_Tuple.c"



#endif