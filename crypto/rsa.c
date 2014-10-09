typedef struct rsaPubKey
{
	mpz_t N;
	mpz_t e;
} rsaPubKey;


typedef struct rsaPrivKey
{
	mpz_t N;
	mpz_t p;
	mpz_t q;
	mpz_t thetaN;
	mpz_t e;
	mpz_t d;
} rsaPrivKey;


struct rsaPrivKey *initPrivKeyRSA()
{
	struct rsaPrivKey *privKey = calloc( 1, sizeof(rsaPrivKey) );
	
	mpz_init(privKey -> N);
	mpz_init(privKey -> p);
	mpz_init(privKey -> q);
	mpz_init(privKey -> thetaN);
	mpz_init(privKey -> e);
	mpz_init(privKey -> d);

	return privKey;
}


struct rsaPubKey *initPubKeyRSA()
{
	struct rsaPubKey *pubKey = calloc( 1, sizeof(rsaPubKey) );
	
	mpz_init(pubKey -> N);
	mpz_init(pubKey -> e);

	return pubKey;
}


void generateRSAKey(struct rsaPrivKey *privKey, struct rsaPubKey *pubKey, gmp_randstate_t state)
{
	mpz_t pMinus1;
	mpz_t qMinus1;

	mpz_init(pMinus1);
	mpz_init(qMinus1);

	privKey = initPrivKeyRSA();
	pubKey = initPubKeyRSA();

	getPrimeGMP(privKey -> p, state, 1024);
	getPrimeGMP(privKey -> q, state, 1024);

	mpz_sub_ui(pMinus1, privKey -> p, 1);
	mpz_sub_ui(qMinus1, privKey -> q, 1);

	mpz_mul(privKey -> N, privKey -> p, privKey -> q);
	mpz_mul(privKey -> thetaN, pMinus1, qMinus1);

	mpz_urandomm(privKey -> e, state, privKey -> thetaN);
    mpz_invert(privKey -> d, privKey -> e, privKey -> thetaN);
}


void testRSA()
{	
	struct rsaPrivKey *privKey;
	struct rsaPubKey *pubKey;

    gmp_randstate_t state;
    unsigned long int seed = time(NULL);
    gmp_randinit_default(state);
    gmp_randseed_ui(state, seed);
	
	generateRSAKey(privKey, pubKey, state);
}