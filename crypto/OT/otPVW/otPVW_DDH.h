// An implementation of the DDH realisation of Dual mode system laid out in...
// "A framework for Efficient and Composable Oblivious Transfer", Peikert, Vaikuntanthan and Waters. 
// CRYPTO 2008

#ifndef PVM_DDH_OT
#define PVM_DDH_OT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../../comms/sockets.h"
#include "../../DDH_Primitive.h"



typedef struct CRS
{
	mpz_t g_0;
	mpz_t g_1;
	mpz_t h_0;
	mpz_t h_1;
} CRS;

typedef struct TrapdoorMessy
{
	mpz_t x_0;
	mpz_t x_1;
} TrapdoorMessy;

typedef struct PVM_OT_PK
{
	mpz_t g;
	mpz_t h;
} PVM_OT_PK;

typedef mpz_t  TrapdoorDec;
typedef mpz_t  PVM_OT_SK;

typedef struct TrapdoorDecKey
{
	struct PVM_OT_PK *pk;
	mpz_t r;
	mpz_t r_yInv;
} TrapdoorDecKey;

typedef struct messyParams
{
	struct CRS *crs;
	struct DDH_Group *group;
	struct TrapdoorMessy *trapdoor;
} messyParams;

typedef struct decParams
{
	struct CRS *crs;
	struct DDH_Group *group;
	TrapdoorDec *trapdoor;
} decParams;

typedef struct otKeyPair
{
	struct PVM_OT_PK *pk;
	PVM_OT_SK *sk;
} otKeyPair;



#include "otPVW_DDH_utils.c"
#include "otPVW_DDH.c"


#endif
