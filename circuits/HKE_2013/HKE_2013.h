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

	// iMax is the number of circuits
	// jMax is the number of inputs
	int numCircuits;
	int numInputs;
} builderInputCommitStruct;


typedef struct secureEqualityCommitments
{
	struct commit_batch_params *params;

	struct elgamal_commit_box **c_boxes_0;
	struct elgamal_commit_box **c_boxes_1;

	struct elgamal_commit_key **k_boxes_0;
	struct elgamal_commit_key **k_boxes_1;
} secureEqualityCommitments;


typedef struct jSetRevealHKE
{
	mpz_t **aListRevealed;
	mpz_t **outputWireShares;

	ub4 **revealedSeeds;

	struct eccPoint ***builderInputsEval;
	mpz_t **logarithms;
	unsigned char *builderInputBits;
} jSetRevealHKE;



#include "HKE_2013_Commitments.c"
#include "HKE_2013_Utils.c"
#include "HKE_2013_Output_Determination.c"
#include "HKE_2013_OT.c"
#include "HKE_2013.c"



#endif