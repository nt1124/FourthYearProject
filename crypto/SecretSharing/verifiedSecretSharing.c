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


struct sharingScheme *init_VSS_SharingScheme(int t, int n)
{
	struct sharingScheme *scheme = (struct sharingScheme *) calloc(1, sizeof(struct sharingScheme));
	int i;


	scheme -> pub = init_VSS_PublicBox(t, n);
	mpz_init(scheme -> secret);

	scheme -> shares = (mpz_t *) calloc(n, sizeof(mpz_t));
	for(i = 0; i < n; i ++)
	{
		mpz_init(scheme -> shares[i]);
	}



	return scheme;
}


struct Fq_poly *getPolyForScheme(int t, mpz_t secret, gmp_randstate_t state, mpz_t q)
{
	struct Fq_poly *toReturn = (struct Fq_poly*) calloc(1, sizeof(struct Fq_poly));
	int i;


	toReturn -> degree = t;
	toReturn -> coeffs = (mpz_t*) calloc(t + 1, sizeof(mpz_t));


	mpz_init_set(toReturn -> coeffs[0], secret);
	for(i = 1; i <= t; i ++)
	{
		mpz_init(toReturn -> coeffs[i]);
		mpz_urandomm(toReturn -> coeffs[i], state, q);
	}

	return toReturn;
}



struct sharingScheme *VSS_Share(mpz_t secret, int t, int n, gmp_randstate_t state, mpz_t q)
{
	struct sharingScheme *scheme = init_VSS_SharingScheme(t, n);
	mpz_t *tempMPZ, secret_index;
	int i;


	mpz_init(secret_index);
	scheme -> poly = getPolyForScheme(t, secret, state, q);

	for(i = 0 ; i < n; i ++)
	{
		mpz_set_ui(secret_index, i + 1);
		tempMPZ = evalutePoly(scheme -> poly, secret_index, q);
		mpz_set(scheme -> shares[i], *tempMPZ);
		free(tempMPZ);
	}

	return scheme;
}