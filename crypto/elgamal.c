

typedef struct elgamalPubKey
{
    mpz_t groupSize;
    mpz_t h;
    mpz_t g;
}elgamalPubKey;


struct elgamalPubKey *initPubKeyElgamal()
{
    struct elgamalPubKey *pubKey = (struct elgamalPubKey*) calloc(1, sizeof(struct elgamalPubKey));

    mpz_init(pubKey -> groupSize);
    mpz_init(pubKey -> g);
    mpz_init(pubKey -> h);

    return pubKey;
}


// Returns the private key.
struct elgamalPubKey *generateKeyPair(mpz_t privKey, int num_bits, gmp_randstate_t state)
{
    struct elgamalPubKey *pubKey = initPubKeyElgamal();

    getPrimeGMP(pubKey -> groupSize, state, num_bits);

    mpz_urandomm(pubKey -> g, state, pubKey -> groupSize);
    mpz_urandomm(privKey, state, pubKey -> groupSize);

    mpz_powm(pubKey -> h, pubKey -> g, privKey, pubKey -> groupSize);

    return pubKey;
}


void updateKeyPair(struct elgamalPubKey *pubKey, mpz_t privKey, int num_bits, gmp_randstate_t state)
{
    getPrimeGMP(pubKey -> groupSize, state, num_bits);

    mpz_urandomm(privKey, state, pubKey -> groupSize);
    mpz_powm(pubKey -> h, pubKey -> g, privKey, pubKey -> groupSize);
}


mpz_t *encElgamal(mpz_t inputMsg, struct elgamalPubKey *pubKey, gmp_randstate_t state)
{
    mpz_t *outputCT = (mpz_t*) calloc(2, sizeof(mpz_t));
    mpz_t ephemeralRand;
    
    mpz_init(outputCT[0]);
    mpz_init(outputCT[1]);
    mpz_init(ephemeralRand);

    do
    {
        mpz_urandomm(ephemeralRand, state, pubKey -> groupSize);
    } while(0 == mpz_cmp_ui (ephemeralRand, 0) );

    mpz_powm(outputCT[0], pubKey -> g, ephemeralRand, pubKey -> groupSize);
    mpz_powm(outputCT[1], pubKey -> h, ephemeralRand, pubKey -> groupSize);

    mpz_mul(outputCT[1], outputCT[1], inputMsg);
    mpz_mod(outputCT[1], outputCT[1], pubKey -> groupSize);

    mpz_clear(ephemeralRand);

    return outputCT;
}


mpz_t *decElgamal(mpz_t *inputCT, mpz_t privKey, struct elgamalPubKey *pubKey)
{
    mpz_t c1PowX, c1PowXInv;
    mpz_t *outputPT = (mpz_t*) calloc(1, sizeof(mpz_t));

    mpz_init(outputPT[0]);
    mpz_init(c1PowX);
    mpz_init(c1PowXInv);

    mpz_powm(c1PowX, inputCT[0], privKey, pubKey -> groupSize);
    mpz_invert(c1PowXInv, c1PowX, pubKey -> groupSize);

    mpz_mul(outputPT[0], c1PowXInv, inputCT[1]);
    mpz_mod(outputPT[0], outputPT[0], pubKey -> groupSize);

    mpz_clear(c1PowX);
    mpz_clear(c1PowXInv);

    return outputPT;
}


void testElgamal()
{
    const int keySize = 512;
    struct elgamalPubKey *pubKey;
    mpz_t privKey, inputMsg, *PT;
    mpz_t *CT;
    int i;

    mpz_init(privKey);
    mpz_init(inputMsg);

    gmp_randstate_t state;
    unsigned long int seed = time(NULL);
    gmp_randinit_default(state);
    gmp_randseed_ui(state, seed);


    pubKey = generateKeyPair(privKey, keySize, state);
    mpz_urandomm(inputMsg, state, pubKey -> groupSize);

    for(i = 0; i < 20; i ++)
    {
        CT = encElgamal(inputMsg, pubKey, state);
        PT = decElgamal(CT, privKey, pubKey);

        if(0 == mpz_cmp(*PT, inputMsg))
            printf("Success : %d\n", i);
        else
            printf("Fail    : %d\n", i);

        updateKeyPair(pubKey, privKey, keySize, state);
    }
}
