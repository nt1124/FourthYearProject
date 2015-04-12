#ifndef CUT_AND_CHOOSE_OT_ECC
#define CUT_AND_CHOOSE_OT_ECC


typedef struct CRS_CnC_ECC
{
	struct eccPoint *g_1;

	// Statistical security parameter (num of circuits).
	int stat_SecParam;

	// J_set SHOULD NOT BE SENT TO THE SENDER.
	// ALSO J_set should be pre-generated, not generated during init of protocol.
	unsigned char *J_set;
	mpz_t *alphas_List;

	struct eccPoint **h_0_List;
	struct eccPoint **h_1_List;
} CRS_CnC_ECC;



typedef struct params_CnC_ECC
{
	struct CRS_CnC_ECC *crs;
	struct eccParams *params;
	
	// y SHOULD NOT BE SENT TO SENDER.
	mpz_t y;
} params_CnC_ECC;


#include "ot_LP_ECC_CnC_utils.c"
#include "ot_LP_ECC_CnC.c"


#endif