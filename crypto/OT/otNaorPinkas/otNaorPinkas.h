#ifndef OT_NP
#define OT_NP


typedef struct OT_NP_Receiver_Query
{
	struct eccPoint *h;
	mpz_t k;
} OT_NP_Receiver_Query;


typedef struct OT_NP_Sender_Transfer
{
	struct eccPoint *a;
	unsigned char *c_0;
	unsigned char *c_1;
} OT_NP_Sender_Transfer;




#include "otNaorPinkasUtils.c"
#include "otNaorPinkas.c"



#endif