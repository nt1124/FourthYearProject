#ifndef CUT_AND_CHOOSE_OT_MOD
#define CUT_AND_CHOOSE_OT_MOD


typedef struct tildeList
{
	struct eccPoint *g_tilde;

	struct eccPoint **h_tildeList;
} tildeList;


typedef struct tildeCRS
{
	mpz_t *r_List;

	struct tildeList **lists;
} tildeCRS;



#include "otL_Mod_CnC_utils.c"
#include "otL_Mod_CnC.c"


#endif