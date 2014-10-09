#ifndef BN_UTIL
#define BN_UTIL

void status() {}


BN_CTX *initBignumContext()
{
    BN_CTX *ctx = BN_CTX_new();

    BN_CTX_init(ctx);

    return ctx;
}


void closeBignumContext(BN_CTX *ctx)
{
    BN_CTX_end(ctx);
}

// ----------------------------------------------------------------------------
// Prime generation code credited to Paolo Ardoino  < paolo.ardoino@gmail.com >
// ----------------------------------------------------------------------------

void print_prime(BIGNUM *outputBN)
{
    char *prime = (char *)malloc(BN_num_bytes(outputBN));
    prime = BN_bn2dec(outputBN);

    int i;
    for(i = 0; i < strlen(prime) && prime[i] == '0'; i++);

    for(; i < strlen(prime); i++)
    {
        printf("%c", prime[i]);
    }
    printf("\n");

    free(prime);
}


BIGNUM *getPrime(long int num_bits)
{
    BIGNUM *outputBN;

    outputBN = BN_new();
    BN_generate_prime(outputBN, num_bits, 1, NULL, NULL, status, NULL);

    return outputBN;
}




typedef struct elgamalPubKeyBN
{
    BIGNUM *groupSize;
    BIGNUM *h;
    BIGNUM *g;
}elgamalPubKeyBN;


struct elgamalPubKeyBN *initPubKeyElgamalBN()
{
    struct elgamalPubKeyBN *pubKey = calloc(1, sizeof(struct elgamalPubKeyBN));

    pubKey -> groupSize = BN_new();
    pubKey -> h = BN_new();
    pubKey -> g = BN_new();

    return pubKey;
}


// Returns the private key.
struct elgamalPubKeyBN *generateKeyPairBN(BIGNUM *privKey, int num_bits, BN_CTX *ctx)
{
    struct elgamalPubKeyBN *pubKey = initPubKeyElgamalBN();
    pubKey -> groupSize = getPrime(num_bits);

    BN_rand_range(pubKey -> g, pubKey -> groupSize);
    BN_rand_range(privKey, pubKey -> groupSize);

    BN_mod_exp(pubKey -> h, pubKey -> g, privKey, pubKey -> groupSize, ctx);

    return pubKey;
}


// Returns the private key.
void updateKeyPairBN(struct elgamalPubKeyBN *pubKey, BIGNUM *privKey, int num_bits, BN_CTX *ctx)
{
    pubKey -> groupSize = getPrime(num_bits);

    BN_rand_range(privKey, pubKey -> groupSize);
    BN_mod_exp(pubKey -> h, pubKey -> g, privKey, pubKey -> groupSize, ctx);
}


BIGNUM **encElgamalBN(BIGNUM *inputMsg, struct elgamalPubKeyBN *pubKey, BN_CTX *ctx)
{
    BIGNUM **outputCT = calloc(2, sizeof(BIGNUM*));
    outputCT[0] = BN_new();
    outputCT[1] = BN_new();

    BIGNUM *ephemeralRand = BN_new();

    do
    {
        BN_rand_range(ephemeralRand, pubKey -> groupSize);
    } while( BN_is_zero(ephemeralRand) );

    BN_mod_exp(outputCT[0], pubKey -> g, ephemeralRand, pubKey -> groupSize, ctx);
    BN_mod_exp(outputCT[1], pubKey -> h, ephemeralRand, pubKey -> groupSize, ctx);
    BN_mod_mul(outputCT[1], outputCT[1], inputMsg, pubKey -> groupSize, ctx);

    BN_free(ephemeralRand);

    return outputCT;
}


BIGNUM *decElgamalBN(BIGNUM **inputCT, BIGNUM *privKey, struct elgamalPubKeyBN *pubKey, BN_CTX *ctx)
{
    BIGNUM *outputPT = BN_new();
    BIGNUM *c1PowX = BN_new();
    BIGNUM *c1PowXInv = BN_new();

    BN_mod_exp(c1PowX, inputCT[0], privKey, pubKey -> groupSize, ctx);
    BN_mod_inverse(c1PowXInv, c1PowX, pubKey -> groupSize, ctx);

    BN_free(c1PowX);
    BN_free(c1PowXInv);
    
    BN_mod_mul(outputPT, c1PowXInv, inputCT[1], pubKey -> groupSize, ctx);

    return outputPT;
}


void testElgamalBN()
{
    const int keySize = 512;
    struct elgamalPubKeyBN *pubKey;
    BIGNUM *privKey = BN_new();
    BIGNUM *inputMsg = BN_new();
    BN_CTX *ctx = initBignumContext();
    BIGNUM **CT;
    BIGNUM *PT;
    int i;


    pubKey = generateKeyPairBN(privKey, keySize, ctx);
    BN_rand_range(inputMsg, pubKey -> groupSize);

    for(i = 0; i < 20; i ++)
    {
        printf("%d\n", i);
        // print_prime(privKey);
        CT = encElgamalBN(inputMsg, pubKey, ctx);
        PT = decElgamalBN(CT, privKey, pubKey, ctx);

        updateKeyPairBN(pubKey, privKey, keySize, ctx);
    }

    closeBignumContext(ctx);
}


#endif