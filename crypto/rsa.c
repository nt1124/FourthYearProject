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
	struct rsaPrivKey *privKey = (struct rsaPrivKey *) calloc( 1, sizeof(rsaPrivKey) );
	
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
	struct rsaPubKey *pubKey = (struct rsaPubKey *)calloc( 1, sizeof(rsaPubKey) );
	
	mpz_init(pubKey -> N);
	mpz_init(pubKey -> e);

	return pubKey;
}


struct rsaPrivKey *generatePrivRSAKey(gmp_randstate_t state)
{
	struct rsaPrivKey *privKey;
	mpz_t pMinus1, qMinus1, gcdThetaNE;

	mpz_init(gcdThetaNE);
	mpz_init(pMinus1);
	mpz_init(qMinus1);

	privKey = initPrivKeyRSA();

	getPrimeGMP(privKey -> p, state, 1023);
	getPrimeGMP(privKey -> q, state, 1023);

	mpz_sub_ui(pMinus1, privKey -> p, 1);
	mpz_sub_ui(qMinus1, privKey -> q, 1);

	mpz_mul(privKey -> N, privKey -> p, privKey -> q);
	mpz_mul(privKey -> thetaN, pMinus1, qMinus1);

	do
	{
		mpz_urandomm(privKey -> e, state, privKey -> thetaN);
		mpz_gcd(gcdThetaNE, privKey -> e, privKey -> thetaN);
    } while( mpz_cmp_ui(gcdThetaNE, 1) );

    mpz_invert(privKey -> d, privKey -> e, privKey -> thetaN);

    return privKey;
}


struct rsaPubKey *generatePubRSAKey(struct rsaPrivKey *privKey)
{
	struct rsaPubKey *pubKey;

	pubKey = initPubKeyRSA();
	
	mpz_set(pubKey -> N, privKey -> N);
	mpz_set(pubKey -> e, privKey -> e);	

    return pubKey;
}


struct rsaPrivKey *updateRSAKey(struct rsaPrivKey *privKey, struct rsaPubKey *pubKey, gmp_randstate_t state)
{
	mpz_t gcdThetaNE;
	mpz_init(gcdThetaNE);

	do
	{
		mpz_urandomm(privKey -> e, state, privKey -> thetaN);
		mpz_gcd(gcdThetaNE, privKey -> e, privKey -> thetaN);
    } while( mpz_cmp_ui(gcdThetaNE, 1) );

    mpz_invert(privKey -> d, privKey -> e, privKey -> thetaN);

    mpz_set(pubKey -> N, privKey -> N);
    mpz_set(pubKey -> e, privKey -> e);

    mpz_clear(gcdThetaNE);
}


mpz_t *encRSA(mpz_t inputPT, struct rsaPubKey *pubKey)
{
	mpz_t *cipherText = (mpz_t*) calloc(1, sizeof(mpz_t));
    mpz_init(*cipherText);

    mpz_powm(*cipherText, inputPT, pubKey -> e, pubKey -> N);

    return cipherText;
}


mpz_t *decRSA(mpz_t inputCT, struct rsaPrivKey *privKey)
{
	mpz_t *plaintext = (mpz_t*) calloc(1, sizeof(mpz_t));
    mpz_init(*plaintext);

    mpz_powm(*plaintext, inputCT, privKey -> d, privKey -> N);

    return plaintext;
}


void testRSA()
{	
	struct rsaPrivKey *privKey;
	struct rsaPubKey *pubKey;
    mpz_t inputMsg, *PT, *CT;
    int i;


    gmp_randstate_t state;
    unsigned long int seed = time(NULL);
    gmp_randinit_default(state);
    gmp_randseed_ui(state, seed);

	privKey = generatePrivRSAKey(state);
	pubKey = generatePubRSAKey(privKey);
    mpz_init(inputMsg);
    mpz_urandomm(inputMsg, state, pubKey -> N);

	for(i = 0; i < 20; i ++)
    {
        CT = encRSA(inputMsg, pubKey);
        PT = decRSA(*CT, privKey);

        if(0 == mpz_cmp(*PT, inputMsg))
            printf("Success : %d\n", i);
        else
            printf("Fail    : %d\n", i);

        updateRSAKey(privKey, pubKey, state);
    }
}