struct CRS *initCRS()
{
	struct CRS *crs = (struct CRS*) calloc(1, sizeof(struct CRS));

	mpz_init(crs -> g_0);
	mpz_init(crs -> g_1);
	mpz_init(crs -> h_0);
	mpz_init(crs -> h_1);

	return crs;
}

struct TrapdoorMessy *initTrapdoorMessy()
{
	struct TrapdoorMessy *t = (struct TrapdoorMessy*) calloc(1, sizeof(struct TrapdoorMessy));

	mpz_init(t -> x_0);
	mpz_init(t -> x_1);

	return t;
}


/*
void precompute(struct DDH_Group *group, mpz_t g0, mpz_t g1, mpz_t h0, mpz_t h1, gmp_randstate_t *state)
{
	group = (struct DDH_Group*) calloc(1, sizeof(struct DDH_Group));

	mpz_init(group -> p);
	mpz_init(group -> g);

	getPrimeGMP(group -> p, state, 1024);
	mpz_urandomm(group -> g, state, group -> p);

	CRS -> g_0 = g0;
	CRS -> g_1 = g1;
	CRS -> h_0 = h0;
	CRS -> h_1 = h1;
}
*/

// Generates a group 
void setupMessy(struct CRS *crs, struct TrapdoorMessy *tMessy,
				int securityParam, struct DDH_Group *group,
				gmp_randstate_t state)
{
	crs = initCRS();
	tMessy = initTrapdoorMessy();

	group = generateGroup(securityParam, state);

	do
	{
		mpz_urandomm(crs -> g_0, state, group -> p);
	} while( 0 < mpz_cmp_ui(crs -> g_0, 1) );

	do
	{
		mpz_urandomm(crs -> g_1, state, group -> p);
	} while( 0 < mpz_cmp_ui(crs -> g_1, 1) );
	
	do
	{
		mpz_urandomm(tMessy -> x_0, state, group -> p);
	} while( 0 != mpz_cmp_ui(tMessy -> x_0, 0) );

	do
	{
		mpz_urandomm(tMessy -> x_1, state, group -> p);
	} while( 0 != mpz_cmp_ui(tMessy -> x_1, 0) &&
			 0 == mpz_cmp(tMessy -> x_0, tMessy -> x_1) );

	mpz_powm(crs -> h_0, crs -> g_0, tMessy -> x_0, group -> p);
	mpz_powm(crs -> h_1, crs -> g_1, tMessy -> x_1, group -> p);
}


void setupDec(struct CRS *crs, TrapdoorDec *t,
			int securityParam,
			struct DDH_Group *group, gmp_randstate_t state)
{
	mpz_t x, y;

	crs = initCRS();
	t = (TrapdoorDec*) calloc(1, sizeof(TrapdoorDec));

	group = generateGroup(securityParam, state);
	mpz_init(x);
	mpz_init(y);
	mpz_init(*t);

	do
	{
		mpz_urandomm(crs -> g_0, state, group -> p);
	} while( 0 < mpz_cmp_ui(crs -> g_0, 1) );

	do
	{
		mpz_urandomm(x, state, group -> p);
	} while( 0 < mpz_cmp_ui(x, 1) );

	do
	{
		mpz_urandomm(*t, state, group -> p);
	} while( 0 != mpz_cmp_ui(*t, 0) );

	mpz_powm(crs -> g_1, crs -> g_0, *t, group -> p);

	mpz_powm(crs -> h_0, crs -> g_0, x, group -> p);
	mpz_powm(crs -> h_1, crs -> g_1, x, group -> p);
}


void keyGen(struct PVM_OT_PK *pk, PVM_OT_SK *sk,
			struct CRS *crs, unsigned char sigmaBit,
			struct DDH_Group *group, gmp_randstate_t state)
{
	pk = (struct PVM_OT_PK*) calloc(1, sizeof(struct PVM_OT_PK));
	mpz_init(pk -> g);
	mpz_init(pk -> h);

	sk = (PVM_OT_SK*) calloc(1, sizeof(PVM_OT_SK));
	mpz_init(*sk);

	do
	{
		mpz_urandomm(*sk, state, group -> p);
	} while( 0 != mpz_cmp_ui(*sk, 0) );

	// Potential to change crs to {g[2], h[2]} and then avoid branching.
	if(0 == sigmaBit)
	{
		mpz_powm(pk -> g, crs -> g_0, *sk, group -> p);
		mpz_powm(pk -> h, crs -> h_0, *sk, group -> p);
	}
	else if(1 == sigmaBit)
	{
		mpz_powm(pk -> g, crs -> g_1, *sk, group -> p);
		mpz_powm(pk -> h, crs -> h_1, *sk, group -> p);
	}
}


struct DDH_PK *setPrimitivePK(struct CRS *crs, struct PVM_OT_PK *otPK, int sigmaBit)
{
	struct DDH_PK *pk = initPublicKey();

	mpz_init(pk -> g);
	mpz_init(pk -> h);

	mpz_init_set(pk -> g_x, otPK -> g);
	mpz_init_set(pk -> h_x, otPK -> h);

	if(0 == sigmaBit)
	{
		mpz_set(pk -> g, crs -> g_0);
		mpz_set(pk -> h, crs -> h_0);
	}
	else
	{
		mpz_set(pk -> g, crs -> g_1);
		mpz_set(pk -> h, crs -> h_1);
	}

	return pk;
}


// Could we speed up by passing in the DDH_PK Blue Peter-ed?
struct u_v_Pair *PVW_OT_Enc(struct CRS *crs, struct PVM_OT_PK *otPK, unsigned char sigmaBit, mpz_t M,
							struct DDH_Group *group, gmp_randstate_t state)
{
	struct u_v_Pair *CT = init_U_V();
	struct DDH_PK *pk = setPrimitivePK(crs, otPK, sigmaBit);

	CT = encDDH(pk, group, M, state);

	return CT;
}


mpz_t *PVW_OT_Dec(struct CRS *crs, PVM_OT_SK *sk, struct u_v_Pair *CT,
							struct DDH_Group *group)
{
	mpz_t *M_Prime = decDDH(sk, group, CT);

	return M_Prime;
}




/*  SENDER
 *  Runs the transfer phase of the OT protocol
 *  ------------------------------------------
 *	Transfer Phase (with inputs x0, x1)
 *	WAIT for message from R
 *	DENOTE the values received by (g,h) 
 *	COMPUTE (u0,v0) = RAND(g0, g, h0, h)
 *	COMPUTE (u1,v1) = RAND(g1, g, h1, h)
 *	COMPUTE c0 = x0 * v0
 *	COMPUTE c1 = x1 * v1
 *	SEND (u0, c0) and (u1, c1) to R
 *	OUTPUT nothing
*/

void senderOT_UC(int writeSocket, unsigned char *input0Bytes, unsigned char *input1Bytes, int inputLengths)
{

}


/*  RECEIVER
 *  Runs the transfer phase of the OT protocol
 *  ------------------------------------------
 *  Transfer Phase (with input sigma)  
 * 	SAMPLE a random value r <- {0, . . . , q-1} 
 * 	COMPUTE
 * 	4.	g = (gSigma) ^ r
 * 	5.	h = (hSigma) ^ r
 * 	SEND (g, h) to S
 * 	WAIT for messages (u0,c0) and (u1,c1) from S
 * 	IF  NOT	(u0, u1, c0, c1) in G
 * 		REPORT ERROR
 * 	OUTPUT  xSigma = cSigma * (uSigma)^(-r)
*/

unsigned char *receiverOT_UC(gmp_randstate_t *state, int readSocket, int inputBit, int *outputLength)
{

}
