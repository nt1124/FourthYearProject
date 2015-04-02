#ifndef GARBLED_CIRCUIT_FROM_RAW_HKE_2013
#define GARBLED_CIRCUIT_FROM_RAW_HKE_2013



typedef struct HKE_Output_Struct_Builder
{
	mpz_t *s_0Array;
	mpz_t *s_1Array;

	struct sharingScheme **scheme0Array;
	struct sharingScheme **scheme1Array;
} HKE_Output_Struct_Builder;


typedef struct HKE_Output_Struct_Exec
{
	mpz_t *s_0Array;
	mpz_t *s_1Array;

	struct sharingScheme **scheme0Array;
	struct sharingScheme **scheme1Array;
} HKE_Output_Struct_Exec;


typedef struct HKE_Commit_Pair_Builder
{

} HKE_Commit_Pair_Builder;



#include "fileUtils_FromRaw_HKE_2013.c"


#endif