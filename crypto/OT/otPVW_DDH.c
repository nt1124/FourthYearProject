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


struct messyParams *initMessyParams()
{
	struct messyParams *params = (struct messyParams*) calloc(1, sizeof(struct messyParams));

	params -> crs = initCRS();
	params -> group = initGroupStruct();
	params -> trapdoor = initTrapdoorMessy();

	return params;
}


struct decParams *initDecParams()
{
	struct decParams *params = (struct decParams*) calloc(1, sizeof(struct messyParams));

	params -> crs = initCRS();
	params -> group = initGroupStruct();
	params -> trapdoor = (TrapdoorDec*) calloc(1, sizeof(TrapdoorDec));
	mpz_init( *(params -> trapdoor) );

	return params;
}


struct otKeyPair *initKeyPair()
{
	struct otKeyPair *keyPair = (struct otKeyPair*) calloc(1, sizeof(struct otKeyPair));
	
	keyPair -> pk = (PVM_OT_PK*) calloc(1, sizeof(PVM_OT_PK));
	mpz_init(keyPair -> pk -> g);
	mpz_init(keyPair -> pk -> h);

	keyPair -> sk = (PVM_OT_SK*) calloc(1, sizeof(PVM_OT_SK));
	mpz_init( *(keyPair -> sk) );

	return keyPair;
}




// Generates a group 
struct messyParams *setupMessy(int securityParam, gmp_randstate_t state)
{
	struct messyParams *params = initMessyParams();

	params -> group = generateGroup(securityParam, state);

	do
	{
		mpz_urandomm(params -> crs -> g_0, state, params -> group -> p);
	} while( 0 < mpz_cmp_ui(params -> crs -> g_0, 1) );

	do
	{
		mpz_urandomm(params -> crs -> g_1, state, params -> group -> p);
	} while( 0 < mpz_cmp_ui(params -> crs -> g_1, 1) );
	
	do
	{
		mpz_urandomm(params -> trapdoor -> x_0, state, params -> group -> p);
	} while( 0 != mpz_cmp_ui(params -> trapdoor -> x_0, 0) );

	do
	{
		mpz_urandomm(params -> trapdoor -> x_1, state, params -> group -> p);
	} while( 0 != mpz_cmp_ui(params -> trapdoor -> x_1, 0) &&
			 0 == mpz_cmp(params -> trapdoor -> x_0, params -> trapdoor -> x_1) );

	mpz_powm(params -> crs -> h_0, params -> crs -> g_0, params -> trapdoor -> x_0, params -> group -> p);
	mpz_powm(params -> crs -> h_1, params -> crs -> g_1, params -> trapdoor -> x_1, params -> group -> p);

	return params;
}


struct decParams *setupDec(int securityParam, gmp_randstate_t state)
{
	mpz_t x, y;
	struct decParams *params = initDecParams();
	params -> group = generateGroup(securityParam, state);

	mpz_init(x);
	mpz_init(y);

	do
	{
		mpz_urandomm(params -> crs -> g_0, state, params -> group -> p);
	} while( 0 > mpz_cmp_ui(params -> crs -> g_0, 1) );

	do
	{
		mpz_urandomm(x, state, params -> group -> p);
	} while( 0 > mpz_cmp_ui(x, 1) );

	do
	{
		mpz_urandomm(*(params -> trapdoor), state, params -> group -> p);
	} while( 0 == mpz_cmp_ui( *(params -> trapdoor), 0) );


	mpz_powm(params -> crs -> g_1, params -> crs -> g_0, *(params -> trapdoor), params -> group -> p);
	mpz_powm(params -> crs -> h_0, params -> crs -> g_0, x, params -> group -> p);
	mpz_powm(params -> crs -> h_1, params -> crs -> g_1, x, params -> group -> p);


	return params;
}


struct otKeyPair *keyGen(struct CRS *crs, unsigned char sigmaBit,
						struct DDH_Group *group, gmp_randstate_t state)
{
	struct otKeyPair *keyPair = initKeyPair();

	do
	{
		mpz_urandomm( *(keyPair -> sk), state, group -> p);
	} while( 0 == mpz_cmp_ui(*(keyPair -> sk), 0) );

	// CRS is null here. Need to use those structs for messyparams etc.
	// Potential to change crs to {g[2], h[2]} and then avoid branching.
	if(0 == sigmaBit)
	{
		mpz_powm(keyPair -> pk -> g, crs -> g_0, *(keyPair -> sk), group -> p);
		mpz_powm(keyPair -> pk -> h, crs -> h_0, *(keyPair -> sk), group -> p);
	}
	else if(1 == sigmaBit)
	{
		mpz_powm(keyPair -> pk -> g, crs -> g_1, *(keyPair -> sk), group -> p);
		mpz_powm(keyPair -> pk -> h, crs -> h_1, *(keyPair -> sk), group -> p);
	}

	return keyPair;
}


struct DDH_PK *setPrimitivePK(struct CRS *crs,
							struct PVM_OT_PK *otPK, int sigmaBit)
{
	struct DDH_PK *pk = initPublicKey();


	mpz_set(pk -> g_x, otPK -> g);
	mpz_set(pk -> h_x, otPK -> h);

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
	struct otKeyPair *keyPair;
	
	unsigned char *input0Bytes = generateRandBytes(16, 16);
	unsigned char *input1Bytes = generateRandBytes(16, 16);
	unsigned char *outputBytes1, *outputBytes2;
	int tempInt1, tempInt2;

	gmp_randstate_t *state = seedRandGen();
	struct decParams *params;

	params = setupDec( 1024, *state );

	convertBytesToMPZ(input0, input0Bytes, 16);
	convertBytesToMPZ(input1, input1Bytes, 16);

	keyPair = keyGen(params -> crs, 0x00, params -> group, *state);

	y0 = PVW_OT_Enc(*input0, params -> crs, params -> group, *state, keyPair -> pk, 0x00);
	y1 = PVW_OT_Enc(*input1, params -> crs, params -> group, *state, keyPair -> pk, 0x01);

	decSigma = PVW_OT_Dec(y0, params -> crs, params -> group, keyPair -> sk);

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


int test_DDH_Local()
{
	mpz_t *input0 = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_t *decSigma = (mpz_t*) calloc(1, sizeof(mpz_t));

	struct u_v_Pair *y0;
	struct DDH_Group *group = initGroupStruct();
	struct DDH_KeyPair *keyPair;
	
	unsigned char *outputBytes1;
	int tempInt1, tempInt2, i;

	gmp_randstate_t *state = seedRandGen();

	group = generateGroup(1024, *state);
	mpz_urandomm(*input0, *state, group -> p);
	keyPair = generateKeys(group, *state);

	y0 = encDDH(keyPair -> pk, group, *input0, *state);
	decSigma = decDDH(keyPair -> sk, group, y0);

	printf("-- %d\n", mpz_cmp(*decSigma, *input0));

	return 0;
}
