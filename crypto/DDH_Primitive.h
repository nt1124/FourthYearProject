#ifndef DDH_PRIMS
#define DDH_PRIMS



typedef struct DDH_Group
{
	mpz_t p;		// The group modulus.
	mpz_t q;		// The group's order.
	mpz_t g;		// Generator of group.
}  DDH_Group;


typedef struct DDH_PK
{
	mpz_t g;
	mpz_t h;
	mpz_t g_x;
	mpz_t h_x;
}  DDH_PK;

typedef struct u_v_Pair
{
	mpz_t u;
	mpz_t v;
}  u_v_Pair;


typedef mpz_t  DDH_SK;


typedef struct DDH_KeyPair
{
	struct DDH_PK *pk;
	DDH_SK *sk;
}  DDH_KeyPair;


#include "gmpUtils.c"
#include "../comms/sockets.h"
#include "DDH_Primitive.c"



#endif