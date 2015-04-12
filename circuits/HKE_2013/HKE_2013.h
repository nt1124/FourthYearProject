#ifndef HKE_2013
#define HKE_2013


typedef struct builderInputCommitStruct
{
	struct commit_batch_params *params;

	unsigned char **commitmentPerms;
	struct elgamal_commit_box ***c_boxes_0;
	struct elgamal_commit_box ***c_boxes_1;

	struct elgamal_commit_key ***k_boxes_0;
	struct elgamal_commit_key ***k_boxes_1;

	int iMax;
	int jMax;
} builderInputCommitStruct;


typedef struct jSetRevealHKE
{
	mpz_t **aListRevealed;
	// mpz_t **shares0Revealed;
	// mpz_t **shares1Revealed;
	mpz_t **ouputWireShares;

	ub4 **revealedSeeds;
} jSetRevealHKE;



#include "HKE_2013_Commitments.c"
#include "HKE_2013_Utils.c"
#include "HKE_2013_OT.c"
#include "HKE_2013.c"



#endif