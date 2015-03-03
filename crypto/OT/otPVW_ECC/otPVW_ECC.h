// An implementation of the DDH_ECC realisation of Dual mode system laid out in...
// "A framework for Efficient and Composable Oblivious Transfer", Peikert, Vaikuntanthan and Waters. 
// CRYPTO 2008

#ifndef PVM_ECC_OT
#define PVM_ECC_OT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../../comms/sockets.h"
#include "../../EllipticCurves/ecc.h"



typedef struct CRS_ECC
{
	struct eccPoint *g_0;
	struct eccPoint *g_1;
	struct eccPoint *h_0;
	struct eccPoint *h_1;
} CRS_ECC;

/*
typedef struct TrapdoorMessy_ECC
{
	mpz_t x_0;
	mpz_t x_1;
} TrapdoorMessy_ECC;
*/


typedef struct PVM_OT_PK_ECC
{
	struct eccPoint *g;
	struct eccPoint *h;
} PVM_OT_PK_ECC;

/*
typedef struct TrapdoorDecKey_ECC
{
	struct PVM_OT_PK *pk;
	mpz_t r;
	mpz_t r_yInv;
} TrapdoorDecKey_ECC;
*/

typedef struct messyParams_ECC
{
	struct CRS_ECC *crs;
	struct eccParams *params;
	// struct TrapdoorMessy *trapdoor;
} messyParams_ECC;


typedef struct decParams_ECC
{
	struct CRS_ECC *crs;
	struct eccParams *params;
	// TrapdoorDec *trapdoor;
} decParams_ECC;


typedef struct otKeyPair_ECC
{
	struct PVM_OT_PK_ECC *pk;
	mpz_t sk;
} otKeyPair_ECC;




#include "otPVW_ECC_utils.c"
#include "otPVW_ECC.c"


#endif
