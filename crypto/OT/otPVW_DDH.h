// An implementation of the DDH realisation of Dual mode system laid out in...
// "A framework for Efficient and Composable Oblivious Transfer", Peikert, Vaikuntanthan and Waters. 

#ifndef PVM_DDH_OT
#define PVM_DDH_OT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../../comms/sockets.h"
#include "../DDH_Primitive.h"



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



#include "otPVW_DDH.c"

#endif