#ifndef ZKPoK_EXT_DH_TUPLE
#define ZKPoK_EXT_DH_TUPLE



struct twoDH_Tuples
{
	struct eccPoint **g_0_List;
	struct eccPoint **g_1_List;
	struct eccPoint **h_0_List;
	struct eccPoint **h_1_List;
} twoDH_Tuples;


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


// Stuff goes here.
#include "ZKPoK_ExtDH_Tuple_1Of2_Utils.c"
#include "ZKPoK_ExtDH_Tuple_1Of2.c"
#include "ZKPoK_ExtDH_Tuple.c"


#endif