typedef struct DDH_Group
{
	mpz_t p;		// Basically the group size, should be prime.
	mpz_t pOrder;	// Basically the group size, should be prime.
	mpz_t g;		// Generator of group.
}  DDH_Group;


typedef struct DDH_PK
{
	mpz_t g;
	mpz_t h;
	mpz_t g_x;
	mpz_t h_x;
}  DDH_PK;


typedef mpz_t  DDH_SK;




struct *DDH_Group initGroupStruct()
{
	struct *DDH_Group group = (struct DDH_Group*) calloc(1, sizeof(struct DDH_Group));

	mpz_init(group -> p);
	mpz_init(group -> pOrder);
	mpz_init(group -> g);

	return group;
}

struct *DDK_PK initPublicKey()
{
	struct *DDK_PK pk = (struct DDK_PK*) calloc(1, sizeof(struct DDK_PK));

	mpz_init(pk -> g);
	mpz_init(pk -> h);
	mpz_init(pk -> g_x);
	mpz_init(pk -> h_x);

	return pk;
}


// SecurityParam is the number of bits for p.
struct *DDH_Group generateGroup(int securityParam, gmp_randstate_t state)
{
	struct *DDH_Group group = initGroupStruct();

	getPrimeGMP(group -> p, state, securityParam);
	do
	{
		mpz_urandomm(group -> g, state, group -> p);
	} while( 0 != mpz_cmp_ui(group -> g, 0) );

	mpz_sub_ui(group -> pOrder, group -> p, 1);

	return group;
}


void generateKeys(struct DDH_PK *pk, DDH_SK *sk, gmp_randstate_t state)
{
	struct DDH_PK *pk = initPublicKey();
	DDH_SK *sk = (DDH_SK) calloc(1, sizeof(mpz_t));
	mpz_init(sk);

	mpz_set(pk -> g, group -> g);
	do
	{
		mpz_urandomm(pk -> h, state, group -> p);
	} while( 0 != mpz_cmp_ui(pk -> h, 0) );

	do
	{
		mpz_urandomm(sk, state, group -> p);
	} while( 0 != mpz_cmp_ui(sk, 0) );

}