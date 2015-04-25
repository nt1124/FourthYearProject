#ifndef L_2013_HKE
#define L_2013_HKE


typedef struct jSetReveal_L_2013_HKE
{
	mpz_t **aListRevealed;

	ub4 **revealedSeeds;

	struct eccPoint ***builderInputsEval;
} jSetReveal_L_2013_HKE;


#include "L_2013_HKE_ECC_Builder.c"
#include "L_2013_HKE_ECC_Executor.c"

#include "L_2013_HKE_Check_Builder.c"
#include "L_2013_HKE_Check_Executor.c"

#include "L_2013_HKE.c"

#endif
