#ifndef CUT_AND_CHOOSE_OT
#define CUT_AND_CHOOSE_OT


typedef struct CRS_CnC
{
	mpz_t g_1;

	// Statistical security parameter (num of circuits).
	int stat_SecParam;

	// J_set SHOULD NOT BE SENT TO THE SENDER.
	// ALSO J_set should be pre-generated, not generated during init of protocol.
	unsigned char *J_set;
	mpz_t *alphas_List;

	mpz_t *h_0_List;
	mpz_t *h_1_List;
} CRS_CnC;


typedef struct params_CnC
{
	struct CRS_CnC *crs;
	struct DDH_Group *group;
	
	// y SHOULD NOT BE SENT TO SENDER.
	mpz_t y;
} params_CnC;


#include "ot_LP_CnC_utils.c"
#include "ot_LP_CnC.c"


#endif