#ifndef VERIFIED_SECRET_SHARING
#define VERIFIED_SECRET_SHARING



typedef struct pubVSS_Box
{
	int t;
	int n;

	mpz_t *commitments;
} pubVSS_Box;


typedef struct sharingSchemePublic
{
	struct pubVSS_Box *pub;

	mpz_t *shares;
} sharingSchemePublic;


typedef struct sharingScheme
{
	struct pubVSS_Box *pub;

	mpz_t secret;
	mpz_t *shares;
	struct Fq_poly *poly;
} sharingScheme;



#include "verifiedSecretSharing.c"


#endif