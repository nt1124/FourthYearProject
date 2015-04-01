struct pubVSS_Box *init_VSS_PublicBox(int t, int n)
{
	struct pubVSS_Box *pub = (struct pubVSS_Box *) calloc(1, sizeof(struct pubVSS_Box));
	int i;


	pub -> t = t;
	pub -> n = n;

	pub -> commitments = (mpz_t *) calloc(t + 1, sizeof(mpz_t));
	for(i = 0; i < t + 1; i ++)
	{
		mpz_init(pub -> commitments[i]);
	}


	return pub;
}


struct pubVSS_Box *init_VSS_PublicBoxWithCoeffs(int t, int n, struct Fq_poly *poly, struct DDH_Group *group)
{
	struct pubVSS_Box *pub = (struct pubVSS_Box *) calloc(1, sizeof(struct pubVSS_Box));
	int i;


	pub -> t = t;
	pub -> n = n;

	pub -> commitments = (mpz_t *) calloc(t + 1, sizeof(mpz_t));
	for(i = 0; i < t + 1; i ++)
	{
		mpz_init(pub -> commitments[i]);
		mpz_powm(pub -> commitments[i], group -> g, poly -> coeffs[i], group -> p);
	}


	return pub;
}


struct sharingScheme *init_VSS_SharingScheme(int t, int n)
{
	struct sharingScheme *scheme = (struct sharingScheme *) calloc(1, sizeof(struct sharingScheme));
	int i;


	mpz_init(scheme -> secret);

	scheme -> shares = (mpz_t *) calloc(n, sizeof(mpz_t));

	for(i = 0; i < n; i ++)
	{
		mpz_init(scheme -> shares[i]);
	}


	return scheme;
}


struct Fq_poly *getPolyForScheme(int t, mpz_t secret, gmp_randstate_t state, struct DDH_Group *group)
{
	struct Fq_poly *toReturn = (struct Fq_poly*) calloc(1, sizeof(struct Fq_poly));
	int i;


	toReturn -> degree = t;
	toReturn -> coeffs = (mpz_t*) calloc(t + 1, sizeof(mpz_t));


	mpz_init_set(toReturn -> coeffs[0], secret);
	for(i = 1; i <= t; i ++)
	{
		mpz_init(toReturn -> coeffs[i]);
		mpz_urandomm(toReturn -> coeffs[i], state, group -> q);
	}


	return toReturn;
}


// IMPORTANT NOTE : You need t + 1 many shares to uncover the secret.
struct sharingScheme *VSS_Share(mpz_t secret, int t, int n, gmp_randstate_t state, struct DDH_Group *group)
{
	struct sharingScheme *scheme = init_VSS_SharingScheme(t, n);
	mpz_t *tempMPZ, secret_index;
	int i;


	mpz_init(secret_index);
	scheme -> poly = getPolyForScheme(t, secret, state, group);
	scheme -> pub = init_VSS_PublicBoxWithCoeffs(t, n, scheme -> poly, group);

	for(i = 0 ; i < n; i ++)
	{
		mpz_set_ui(secret_index, i + 1);
		tempMPZ = evalutePoly(scheme -> poly, secret_index, group -> p);
		mpz_set(scheme -> shares[i], *tempMPZ);
		free(tempMPZ);
	}

	return scheme;
}


int VSS_Verify(struct pubVSS_Box *pub, mpz_t shareToVerify, unsigned int index, struct DDH_Group *group)
{
	mpz_t indexMPZ, indexMPZPowT, gPowShare, c_iProduct, c_iTemp, temp;
	int i, verified = 0;


	mpz_init(temp);
	mpz_init(c_iTemp);
	mpz_init_set(c_iProduct, pub -> commitments[0]);
	mpz_init(gPowShare);
	mpz_init_set_ui(indexMPZ, index);
	mpz_init_set(indexMPZPowT, indexMPZ);


	mpz_mod(temp, shareToVerify, group -> q);
	mpz_powm(gPowShare, group -> g, temp, group -> p);

	for(i = 1; i <= pub -> t; i ++)
	{
		mpz_powm_ui(indexMPZPowT, indexMPZ, i, group -> q);
		mpz_powm(c_iTemp, pub -> commitments[i], indexMPZPowT, group -> p);

		mpz_mul(temp, c_iProduct, c_iTemp);
		mpz_mod(c_iProduct, temp, group -> p);
	}

	verified = mpz_cmp(c_iProduct, gPowShare);


	return verified;
}


mpz_t *VSS_Recover(mpz_t *shares, int *indices, int length, struct DDH_Group *group)
{
	struct Fq_poly *secretPoly = getPolyFromCodewords(shares, indices, length, group -> p);
	mpz_t zeroMPZ, *secret;


	mpz_init_set_ui(zeroMPZ, 0);
	secret = evalutePoly(secretPoly, zeroMPZ, group -> q);


	return secret;
}



void testVSS()
{
	struct DDH_Group *group;
	struct sharingScheme *scheme;
	mpz_t secret, *tempMPZ;
	gmp_randstate_t *state;
	unsigned int i;
	int verified = 0, *tempIndices = (int *) calloc(10, sizeof(int));


	state = seedRandGen();
	group = getSchnorrGroup(1024, *state);

	// mpz_set_ui(group -> g, 3);
	// mpz_set_ui(group -> q, 5);
	// mpz_set_ui(group -> p, 11);

	mpz_init(secret);
	mpz_urandomm(secret, *state, group -> q);

	scheme = VSS_Share(secret, 5, 10, *state, group);

	for(i = 0; i < 10; i ++)
	{
		tempIndices[i] = i + 1;
	}

	for(i = 2; i <= 10; i ++)
	{
		tempMPZ = VSS_Recover(scheme -> shares, tempIndices, i, group);
		printf("%d  =>  %d\n", i, mpz_cmp(*tempMPZ, secret));
	}

	printf("\n\n\n");

	for(i = 0; i < 10; i ++)
	{
		verified = VSS_Verify( scheme -> pub, scheme -> shares[i], i + 1, group);
		printf("%d  =>  %d\n", i, verified);
	}
}







