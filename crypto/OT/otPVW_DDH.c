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


struct TrapdoorDecKey *initTrapdoorDecKey()
{
	struct TrapdoorDecKey *tDecKey = (struct TrapdoorDecKey*) calloc(1, sizeof(struct TrapdoorDecKey));

	tDecKey -> pk = (struct PVM_OT_PK*) calloc(1, sizeof(struct PVM_OT_PK));
	mpz_init(tDecKey -> r);
	mpz_init(tDecKey -> r_yInv);

	return tDecKey;
}


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

	printf("Checkpoint A\n");
	fflush(stdout);

	group = generateGroup(securityParam, state);
	mpz_init(x);
	mpz_init(y);
	mpz_init(*t);

	do
	{
		mpz_urandomm(crs -> g_0, state, group -> p);
	} while( 0 > mpz_cmp_ui(crs -> g_0, 1) );

	printf("Checkpoint B\n");
	fflush(stdout);

	do
	{
		mpz_urandomm(x, state, group -> p);
	} while( 0 > mpz_cmp_ui(x, 1) );

	do
	{
		mpz_urandomm(*t, state, group -> p);
	} while( 0 == mpz_cmp_ui(*t, 0) );

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

	// SEG FAULT HERE
	do
	{
		mpz_urandomm(*sk, state, group -> p);
	} while( 0 == mpz_cmp_ui(*sk, 0) );

	printf("Checkpoint Meh\n");
	fflush(stdout);
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


struct DDH_PK *setPrimitivePK(struct CRS *crs,
							struct PVM_OT_PK *otPK, int sigmaBit)
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
struct u_v_Pair *PVW_OT_Enc(mpz_t M, 
							struct CRS *crs, struct DDH_Group *group, gmp_randstate_t state,
							struct PVM_OT_PK *otPK, unsigned char sigmaBit)
{
	struct u_v_Pair *CT = init_U_V();
	struct DDH_PK *pk = setPrimitivePK(crs, otPK, sigmaBit);

	CT = encDDH(pk, group, M, state);

	return CT;
}


mpz_t *PVW_OT_Dec(struct u_v_Pair *CT,
				struct CRS *crs, struct DDH_Group *group,
				PVM_OT_SK *sk)
{
	mpz_t *M_Prime = decDDH(sk, group, CT);

	return M_Prime;
}


unsigned char findMessy(struct CRS *crs, struct DDH_Group *group,
						struct TrapdoorMessy *t, struct PVM_OT_PK *pk)
{
	mpz_t g_x_0;

	if( 0 == mpz_cmp(t -> x_0, t -> x_1) )
	{
		printf("ERROR: x_0 == x_1 in Trapdoor for OT_PVW_DDH\n");
		return 0xFF;
	}

	mpz_init(g_x_0);
	mpz_powm(g_x_0, pk -> g, t -> x_0, group -> p);

	if( 0 != mpz_cmp(g_x_0, pk -> h) )
	{
		return 0x00;
	}

	return 0x01;
}


struct TrapdoorDecKey *trapdoorKeyGeneration( struct CRS *crs, struct DDH_Group *group,
											gmp_randstate_t state, TrapdoorDec *t)
{
	struct TrapdoorDecKey *tDecKey = initTrapdoorDecKey();
	mpz_t yInv;

	mpz_init(yInv);

	mpz_urandomm(tDecKey -> r, state, group -> p);

	mpz_powm(tDecKey -> pk -> g, crs -> g_0, tDecKey -> r, group -> p);
	mpz_powm(tDecKey -> pk -> h, crs -> h_0, tDecKey -> r, group -> p);

	mpz_invert(yInv, *t, group -> p);

	mpz_mul(tDecKey -> r_yInv, tDecKey -> r, yInv);
	mpz_mod(tDecKey -> r_yInv, tDecKey -> r_yInv, group -> p);

	return tDecKey;
}



/*  SENDER
 *  Runs the transfer phase of the OT protocol
 *  ------------------------------------------
 *	Transfer Phase (with inputs x_0, x_1)
 *	WAIT for message from R
 *	DENOTE the values received by (g,h) 
 *	COMPUTE (u_0,v_0) = RAND(g_0, g, h_0, h)
 *	COMPUTE (u_1,v_1) = RAND(g_1, g, h_1, h)
 *	COMPUTE c_0 = x_0 * v_0
 *	COMPUTE c_1 = x_1 * v_1
 *	SEND (u_0, c_0) and (u_1, c_1) to R
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
 * 	4.	g = (g_Sigma) ^ r
 * 	5.	h = (h_Sigma) ^ r
 * 	SEND (g, h) to S
 * 	WAIT for messages (u0,c0) and (u1,c1) from S
 * 	IF  NOT	(u0, u1, c0, c1) in G
 * 		REPORT ERROR
 * 	OUTPUT  xSigma = cSigma * (uSigma)^(-r)
*/

unsigned char *receiverOT_UC(gmp_randstate_t *state, int readSocket, int inputBit, int *outputLength)
{

}





int testOT_PWV_DDH_Local()
{
	mpz_t *input0 = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_t *input1 = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_t *decSigma = (mpz_t*) calloc(1, sizeof(mpz_t));
	struct u_v_Pair *y0, *y1;
	
	unsigned char *input0Bytes = generateRandBytes(16, 16);
	unsigned char *input1Bytes = generateRandBytes(16, 16);
	unsigned char *outputBytes1, *outputBytes2;
	int tempInt1, tempInt2;

	gmp_randstate_t *state = seedRandGen();
	struct DDH_Group *group;
	struct CRS *crs;
	struct PVM_OT_PK *pk;
	PVM_OT_SK *sk;
	TrapdoorDec *t;

	setupDec( crs, t, 1024, group, *state );
	printf("Checkpoint 1\n");
	fflush(stdout);
	convertBytesToMPZ(input0, input0Bytes, 16);
	convertBytesToMPZ(input1, input1Bytes, 16);

	keyGen(pk, sk, crs, 0x00, group, *state);

	printf("Checkpoint 2\n");
	fflush(stdout);

	y0 = PVW_OT_Enc(*input0, crs, group, *state, pk, 0x00);

	printf("Checkpoint 3\n");
	fflush(stdout);

	y1 = PVW_OT_Enc(*input1, crs, group, *state, pk, 0x01);

	printf("Checkpoint 4\n");
	fflush(stdout);

	gmp_printf("%Zd\n", *input0);
	gmp_printf("%Zd\n", *input1);

	decSigma = PVW_OT_Dec(y0, crs, group, sk);

	outputBytes1 = convertMPZToBytes(*decSigma, &tempInt1);
	outputBytes2 = convertMPZToBytes(*input0, &tempInt2);

	if(16 == tempInt1)
		if(strncmp((char*)outputBytes1, (char*)input0Bytes, tempInt1) == 0)
			return 1;

	int i;
	printf("%d = %d\n", 16, tempInt1);
	for(i = 0; i < tempInt1; i ++)
	{	
		printf("%u + %u + %u\n", input0Bytes[i], outputBytes1[i], outputBytes2[i]);
	}

	return 0;
}
