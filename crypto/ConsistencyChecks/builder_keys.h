#ifndef LP_2014_BUILDER_KEYS
#define LP_2014_BUILDER_KEYS


typedef struct secret_builderPRS_Keys
{
	int numKeyPairs;
	mpz_t **secret_keyPairs;

	int stat_SecParam;
	mpz_t *secret_circuitKeys;

} secret_builderPRS_Keys;


typedef struct public_builderPRS_Keys
{
	int numKeyPairs;
	mpz_t **public_keyPairs;

	int stat_SecParam;
	mpz_t *public_circuitKeys;
} public_builderPRS_Keys;


#include "builder_keys.c"


#endif