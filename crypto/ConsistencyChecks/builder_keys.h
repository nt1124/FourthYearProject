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


typedef struct publicInputsWithGroup
{
	struct DDH_Group *group;
	struct public_builderPRS_Keys *public_inputs;
} publicInputsWithGroup;


typedef struct revealedCheckSecrets
{
	mpz_t *revealedSecrets;
	unsigned int *revealedSeeds;
} revealedCheckSecrets;


#include "builder_keys.c"


#endif