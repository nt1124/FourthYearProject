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


typedef struct msgOne_notI_Arrays
{
	int *not_in_I_index;
	mpz_t *C_array;
	mpz_t *Z_array;
} msgOne_notI_Arrays;


typedef struct msgOneArrays
{
	struct msgOne_notI_Arrays *notI_Struct;

	int *in_I_index;
	mpz_t *roeArray;

	mpz_t *A_array;
	mpz_t *B_array;
} msgOneArrays;


/*
typedef struct msgArrays
{

	mpz_t *roeArray;
	mpz_t *C_array;
	mpz_t *Z_array;

	mpz_t *A_array;
	mpz_t *B_array;
} msgArrays;
*/


#include "ZK_PK_DDH_Tuple_utils.c"
#include "ZK_PK_DDH_Tuple.c"



#endif