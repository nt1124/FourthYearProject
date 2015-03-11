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


typedef struct CnC_OT_Mod_CTs
{
	struct eccPoint *u_0;
	unsigned char *w_0;

	struct eccPoint *u_1;
	unsigned char *w_1;
} CnC_OT_Mod_CTs;


typedef struct CnC_OT_Mod_PK
{
	struct eccPoint *u_1;
} CnC_OT_Mod_PK;



#include "otL_Mod_CnC_utils.c"
#include "otL_Mod_CnC_Receiver.c"
#include "otL_Mod_CnC_Sender.c"


#endif