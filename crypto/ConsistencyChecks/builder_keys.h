#ifndef LP_2010_BUILDER_KEYS
#define LP_2010_BUILDER_KEYS


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
	struct eccPoint ***public_keyPairs;

	int stat_SecParam;
	struct eccPoint **public_circuitKeys;
} public_builderPRS_Keys;


typedef struct publicInputsWithGroup
{
	struct eccParams *params;
	struct public_builderPRS_Keys *public_inputs;
} publicInputsWithGroup;


typedef struct revealedCheckSecrets
{
	mpz_t *revealedSecrets;
	unsigned int *revealedSeeds;
	ub4 **revealedCircuitSeeds;
} revealedCheckSecrets;


#include "builder_keys.c"


#endif