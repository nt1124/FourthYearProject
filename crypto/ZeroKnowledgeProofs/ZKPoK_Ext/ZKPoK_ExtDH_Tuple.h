#ifndef ZKPoK_EXT_DH_TUPLE
#define ZKPoK_EXT_DH_TUPLE



struct twoDH_Tuples
{
	struct eccPoint **g_0_List;
	struct eccPoint **g_1_List;
	struct eccPoint **h_0_List;
	struct eccPoint **h_1_List;
} twoDH_Tuples;


// Stuff goes here.
#include "ZKPoK_ExtDH_Tuple_1Of2.c"
#include "ZKPoK_ExtDH_Tuple.c"


#endif