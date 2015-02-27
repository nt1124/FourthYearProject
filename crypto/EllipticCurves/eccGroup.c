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


struct eccPoint *invertPoint(struct eccPoint *P, struct eccParams *params)
{
	struct eccPoint *invP = initECC_Point();


	mpz_set(invP -> x, P -> x);
	mpz_invert(invP -> y, P -> y, params -> p);


	return invP;
}