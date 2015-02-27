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


struct eccPoint *doublePoint(struct eccPoint *P, struct eccParams *params)
{
	struct eccPoint *R = groupOp(P, P, params);

	return R;
}


struct eccPoint *invertPoint(struct eccPoint *P, struct eccParams *params)
{
	struct eccPoint *invP = initECC_Point();


	mpz_set(invP -> x, P -> x);
	mpz_sub(invP -> y, params -> p, P -> y);

	return invP;
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


struct eccPoint *doubleAndAdd_ScalarMul(mpz_t k, struct eccPoint *P, struct eccParams *params)
{
	struct eccPoint *Q = init_Identity_ECC_Point();
	struct eccPoint *tempP = copyECC_Point(P), *temp;
	int numBitsInK = mpz_sizeinbase(k, 2), i;

	for(i = 0; i < numBitsInK; i ++)
	{
		if(1 == mpz_tstbit(k, i))
		{
			temp = groupOp(Q, tempP, params);
			clearECC_Point(Q);
			Q = temp;
		}

		temp = doublePoint(tempP, params);
		clearECC_Point(tempP);
		tempP = temp;
	}

	return Q;
}


struct eccParams *initBrainpool_160_Curve()
{
	const char *pStr = "E95E4A5F737059DC60DFC7AD95B3D8139515620F";
	const char *aStr = "340E7BE2A280EB74E2BE61BADA745D97E8F7C300";
	const char *bStr = "1E589A8595423412134FAA2DBDEC95C8D8675E58";
	const char *g_xStr = "BED5AF16EA3F6A4F62938C4631EB5AF7BDBCDBC3";
	const char *g_yStr = "1667CB477A1A8EC338F94741669C976316DA6321";
	const char *nStr = "E95E4A5F737059DC60DF5991D45029409E60FC09";

	struct eccParams *params = initECC_Params();


	gmp_sscanf(pStr, "%Zx", params -> p);
	gmp_sscanf(aStr, "%Zx", params -> a);
	gmp_sscanf(bStr, "%Zx", params -> b);
	gmp_sscanf(g_xStr, "%Zx", params -> g -> x);
	gmp_sscanf(g_yStr, "%Zx", params -> g -> y);
	gmp_sscanf(nStr, "%Zx", params -> n);


	return params;
}



struct eccParams *initBrainpool_256_Curve()
{
	const char *pStr = "A9FB57DBA1EEA9BC3E660A909D838D726E3BF623D52620282013481D1F6E5377";
	const char *aStr = "7D5A0975FC2C3057EEF67530417AFFE7FB8055C126DC5C6CE94A4B44F330B5D9";
	const char *bStr = "26DC5C6CE94A4B44F330B5D9BBD77CBF958416295CF7E1CE6BCCDC18FF8C07B6";
	const char *g_xStr = "8BD2AEB9CB7E57CB2C4B482FFC81B7AFB9DE27E1E3BD23C23A4453BD9ACE3262";
	const char *g_yStr = "547EF835C3DAC4FD97F8461A14611DC9C27745132DED8E545C1D54C72F046997";
	const char *nStr = "A9FB57DBA1EEA9BC3E660A909D838D718C397AA3B561A6F7901E0E82974856A7";

	struct eccParams *params = initECC_Params();


	gmp_sscanf(pStr, "%ZX", params -> p);
	gmp_sscanf(aStr, "%ZX", params -> a);
	gmp_sscanf(bStr, "%ZX", params -> b);
	gmp_sscanf(g_xStr, "%ZX", params -> g -> x);
	gmp_sscanf(g_yStr, "%ZX", params -> g -> y);
	gmp_sscanf(nStr, "%ZX", params -> n);


	return params;
}








