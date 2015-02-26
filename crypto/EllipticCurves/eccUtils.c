struct eccPoint *initECC_Point()
{
	struct eccPoint *output = (struct eccPoint *) calloc(1, sizeof(struct eccPoint));

	mpz_init(output -> x);
	mpz_init(output -> y);

	output -> pointAtInf = 0;

	return output;
}


struct eccPoint *init_Identity_ECC_Point()
{
	struct eccPoint *output = (struct eccPoint *) calloc(1, sizeof(struct eccPoint));

	mpz_init(output -> x);
	mpz_init(output -> y);

	output -> pointAtInf = 1;

	return output;
}



struct eccPoint *initAndSetECC_Point(mpz_t x, mpz_t y, unsigned char identity)
{
	struct eccPoint *output = (struct eccPoint *) calloc(1, sizeof(struct eccPoint));

	mpz_init_set(output -> x, x);
	mpz_init_set(output -> y, y);

	output -> pointAtInf = identity;


	return output;
}



struct eccPoint *copyECC_Point(struct eccPoint *toCopy)
{
	struct eccPoint *output = (struct eccPoint *) calloc(1, sizeof(struct eccPoint));

	mpz_init_set(output -> x, toCopy -> x);
	mpz_init_set(output -> y, toCopy -> y);

	output -> pointAtInf = toCopy -> pointAtInf;

	return output;
}



void printPoint(struct eccPoint *P)
{
	if(0 == P -> pointAtInf)
	{
		gmp_printf("(%Zd, %Zd)\n", P -> x, P -> y);
	}
	else
	{
		printf("(@, @)\n");
	}
}




struct eccParams *initECC_Params()
{
	struct eccParams *output = (struct eccParams *) calloc(1, sizeof(struct eccParams));

	mpz_init(output -> p);
	mpz_init(output -> a);
	mpz_init(output -> b);
	output -> g = initECC_Point();
	mpz_init(output -> n);

	return output;
}


struct eccParams *initAndSetECC_Params(mpz_t p, mpz_t a, mpz_t b, struct eccPoint *g, mpz_t n)
{
	struct eccParams *output = (struct eccParams *) calloc(1, sizeof(struct eccParams));

	mpz_init_set(output -> p, p);
	mpz_init_set(output -> a, a);
	mpz_init_set(output -> b, b);
	mpz_init_set(output -> n, n);

	output -> g = g;

	return output;
}




int additionDecision(struct eccPoint *P, struct eccPoint *Q)
{
	int toReturn = 0;

	if(P -> pointAtInf == 0x01)
	{
		// R = Q
		toReturn = 0;
	}
	else if(Q -> pointAtInf == 0x01)
	{
		// R = P
		toReturn = 1;
	}
	else if(0 == mpz_cmp(P -> x, Q -> x)  && 0 != mpz_cmp(P -> y, Q -> y))
	{
		// R = (@, @)
		toReturn = 2;
	}
	else if(0 != mpz_cmp(P -> x, Q -> x))
	{
		// R = P * Q
		toReturn = 3;
	}
	else if(0 == mpz_cmp_ui(P -> y, 0) && 0 == mpz_cmp_ui(Q -> y, 0))
	{
		// R = (@, @)
		toReturn = 4;
	}
	else
	{
		// R = P * Q ()
		toReturn = 5;
	}


	return toReturn;
}



struct eccPoint *groupOp_Xs_unequal(struct eccPoint *P, struct eccPoint *Q, struct eccParams *params)
{
	mpz_t topHalf, lowerHalf, lowerHalf_inv, unsquared, squared, temp1, unmodded;
	struct eccPoint *output = initECC_Point();

	mpz_init(topHalf);
	mpz_init(lowerHalf);
	mpz_init(lowerHalf_inv);
	mpz_init(unsquared);
	mpz_init(squared);
	mpz_init(temp1);
	mpz_init(unmodded);

	// x3 = ((y2-y1)/(x2-x1))^2 - x1 - x2
	mpz_sub(topHalf, Q -> y, P -> y);
	mpz_sub(lowerHalf, Q -> x, P -> x);
	mpz_invert(lowerHalf_inv, lowerHalf, params -> p);

	mpz_mul(unsquared, topHalf, lowerHalf_inv);
	mpz_powm_ui(squared, unsquared, 2, params -> p);

	mpz_sub(temp1, squared, P -> x);
	mpz_sub(unmodded, temp1, Q -> x);
	mpz_mod(output -> x, unmodded, params -> p);


	// y3 = (x1-x3)*(y2-y1)/(x2-x1) - y1
	mpz_sub(topHalf, Q -> y, P -> y);
	mpz_sub(lowerHalf, Q -> x, P -> x);
	mpz_invert(lowerHalf_inv, lowerHalf, params -> p);
	mpz_mul(unsquared, topHalf, lowerHalf_inv);

	mpz_sub(topHalf, P -> x, output -> x);
	mpz_mul(temp1, topHalf, unsquared);

	mpz_sub(unmodded, temp1, P -> y);
	mpz_mod(output -> y, unmodded, params -> p);


	return output;
}



struct eccPoint *groupOp_Equal_Y_NotZero(struct eccPoint *P, struct eccPoint *Q, struct eccParams *params)
{
	mpz_t topHalf, lowerHalf, lowerHalf_inv, temp1, temp2, fractionPart, unmodded;
	struct eccPoint *output = initECC_Point();

	mpz_init(topHalf);
	mpz_init(lowerHalf);
	mpz_init(lowerHalf_inv);
	mpz_init(temp1);
	mpz_init(temp2);
	mpz_init(fractionPart);
	mpz_init(unmodded);


	// x3 = ((3*x1^2 + a)/(2*y1))^2 - 2*x1
	mpz_powm_ui(temp1, P -> x, 2, params -> p);
	mpz_mul_ui(temp2, temp1, 3);
	mpz_add(topHalf, temp2, params -> a);

	mpz_mul_ui(lowerHalf, P -> y, 2);
	mpz_invert(lowerHalf_inv, lowerHalf, params -> p);
	mpz_mul(fractionPart, topHalf, lowerHalf_inv);
	mpz_powm_ui(temp2, fractionPart, 2, params -> p);

	mpz_mul_ui(temp1, P -> x, 2);
	mpz_sub(unmodded, temp2, temp1);
	mpz_mod(output -> x, unmodded, params -> p);


	// y3 = (x1-x3)*(3*x1^2 + a)/(2*y1) - y1  (Note that (3*x1^2 + a)/(2*y1) = fractionPart)
	mpz_sub(temp1, P -> x, output -> x);
	mpz_mul(temp2, temp1, fractionPart);
	mpz_sub(unmodded, temp2, P -> y);
	mpz_mod(output -> y, unmodded, params -> p);


	return output;
}



struct eccPoint *groupOp(struct eccPoint *P, struct eccPoint *Q, struct eccParams *params)
{
	struct eccPoint *output;
	int addDecision = additionDecision(P, Q);


	if(3 == addDecision)
	{
		output = groupOp_Xs_unequal(P, Q, params);
	}
	else if(5 == addDecision)
	{
		output = groupOp_Equal_Y_NotZero(P, Q, params);
	}
	else if(0 == addDecision)
	{
		output = copyECC_Point(Q);
	}
	else if(1 == addDecision)
	{
		output = copyECC_Point(P);
	}
	else
	{
		output = initECC_Point();
		output -> pointAtInf = 1;
	}

	return output;
}


struct eccPoint *scalarMulti(mpz_t k, struct eccPoint *P, struct eccParams *params)
{
	struct eccPoint *toFree = init_Identity_ECC_Point();
	struct eccPoint *tempPoint = toFree;
	mpz_t i;


	mpz_init_set_ui(i, 0);

	if(0 == mpz_cmp_ui(k, 0))
		return NULL;


	while(0 > mpz_cmp(i, k))
	{
		tempPoint = groupOp(P, toFree, params);
		free(toFree);
		toFree = tempPoint;

		mpz_add_ui(i, i, 1);
	}


	return tempPoint;
}



int testECC_Utils()
{
	mpz_t p, a, b, n, g_x, g_y, plaintext_x, plaintext_y;
	mpz_t SK_a, SK_b, SK_j;
	struct eccPoint *g, *plaintext, *ciphertext;
	struct eccPoint *PK_a, *PK_b, *PK_j, *PK_for_use;
	struct eccParams *params;

	mpz_t c_x, c_y, temp;

	mpz_init_set_ui(p, 263);
	mpz_init_set_ui(a, 1);
	mpz_init_set_ui(b, 1);
	mpz_init_set_ui(g_x, 219);
	mpz_init_set_ui(g_y, 118);
	mpz_init_set_ui(n, 64);
	mpz_init_set_ui(plaintext_x, 19);
	mpz_init_set_ui(plaintext_y, 72);

	mpz_init_set_ui(SK_a, 103);
	mpz_init_set_ui(SK_b, 205);
	mpz_init_set_ui(SK_j, 209);

	mpz_init(c_x);
	mpz_init(c_y);
	mpz_init(temp);


	g = initAndSetECC_Point(g_x, g_y, 0);
	plaintext = initAndSetECC_Point(plaintext_x, plaintext_y, 0);
	params = initAndSetECC_Params(p, a, b, g, n);


	// Test the public key crap.
	PK_a = scalarMulti(SK_a, g, params);
	printPoint(PK_a);
	PK_b = scalarMulti(SK_b, g, params);
	printPoint(PK_b);
	PK_j = scalarMulti(SK_j, g, params);
	printPoint(PK_j);

	ciphertext = initECC_Point();
	PK_for_use = scalarMulti(SK_a, PK_b, params);
	mpz_mul(temp, plaintext -> x, PK_for_use -> x);
	mpz_mod(ciphertext -> x, temp, params -> p);
	mpz_mul(temp, plaintext -> y, PK_for_use -> y);
	mpz_mod(ciphertext -> y, temp, params -> p);

	printPoint(ciphertext);

	return 0;
}