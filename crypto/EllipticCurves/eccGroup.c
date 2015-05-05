// See this link for how we do Elliptic curve additions and doubles.
// http://en.wikipedia.org/wiki/Elliptic_curve_point_multiplication




// Double a point in place
void doubleGroupOpInPlace(struct eccPoint *P, struct eccParams *params)
{
	mpz_t topHalf, lowerHalf, lowerHalf_inv, temp1, temp2, fractionPart, unmoddedX, unmoddedY;

	mpz_init(topHalf);
	mpz_init(lowerHalf);
	mpz_init(lowerHalf_inv);
	mpz_init(temp1);
	mpz_init(temp2);
	mpz_init(fractionPart);
	mpz_init(unmoddedX);
	mpz_init(unmoddedY);


	// x3 = ((3*x1^2 + a)/(2*y1))^2 - 2*x1
	mpz_powm_ui(temp1, P -> x, 2, params -> p);
	mpz_mul_ui(temp2, temp1, 3);
	mpz_add(topHalf, temp2, params -> a);

	mpz_mul_ui(lowerHalf, P -> y, 2);
	mpz_invert(lowerHalf_inv, lowerHalf, params -> p);
	mpz_mul(fractionPart, topHalf, lowerHalf_inv);
	mpz_mul(temp2, fractionPart, fractionPart);

	mpz_mul_ui(temp1, P -> x, 2);
	mpz_sub(unmoddedX, temp2, temp1);
	mpz_mod(topHalf, unmoddedX, params -> p);


	// y3 = (x1-x3)*(3*x1^2 + a)/(2*y1) - y1  (Note that (3*x1^2 + a)/(2*y1) = fractionPart)
	mpz_sub(temp1, P -> x, topHalf);
	mpz_mul(temp2, temp1, fractionPart);
	mpz_sub(unmoddedY, temp2, P -> y);


	mpz_set(P -> x, topHalf);
	mpz_mod(P -> y, unmoddedY, params -> p);


	// Housekeeping
	mpz_clear(topHalf);
	mpz_clear(lowerHalf);
	mpz_clear(lowerHalf_inv);
	mpz_clear(temp1);
	mpz_clear(temp2);
	mpz_clear(fractionPart);
	mpz_clear(unmoddedX);
	mpz_clear(unmoddedY);
}


// Double a point, Not in place.
struct eccPoint *doubleGroupOp(struct eccPoint *P, struct eccParams *params)
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
	mpz_mul(temp2, fractionPart, fractionPart);

	mpz_mul_ui(temp1, P -> x, 2);
	mpz_sub(unmodded, temp2, temp1);
	mpz_mod(output -> x, unmodded, params -> p);


	// y3 = (x1-x3)*(3*x1^2 + a)/(2*y1) - y1  (Note that (3*x1^2 + a)/(2*y1) = fractionPart)
	mpz_sub(temp1, P -> x, output -> x);
	mpz_mul(temp2, temp1, fractionPart);
	mpz_sub(unmodded, temp2, P -> y);
	mpz_mod(output -> y, unmodded, params -> p);

	mpz_clear(topHalf);
	mpz_clear(lowerHalf);
	mpz_clear(lowerHalf_inv);
	mpz_clear(temp1);
	mpz_clear(temp2);
	mpz_clear(fractionPart);
	mpz_clear(unmodded);

	return output;
}


// Add the ECC points P and Q together.
struct eccPoint *addEC_Point(struct eccPoint *P, struct eccPoint *Q, struct eccParams *params)
{
	mpz_t topHalf, lowerHalf, lowerHalf_inv, temp1, temp2, unmodded, lambda, lambdaSq;
	struct eccPoint *output = initECC_Point();

	mpz_init(topHalf);
	mpz_init(lowerHalf);
	mpz_init(lowerHalf_inv);
	mpz_init(temp1);
	mpz_init(temp2);
	mpz_init(unmodded);
	mpz_init(lambda);
	mpz_init(lambdaSq);

	mpz_sub(topHalf, Q -> y, P -> y);
	mpz_sub(lowerHalf, Q -> x, P -> x);
	mpz_invert(lowerHalf_inv, lowerHalf, params -> p);
	mpz_mul(lambda, topHalf, lowerHalf_inv);

	mpz_powm_ui(lambdaSq, lambda, 2, params -> p);
	mpz_sub(temp1, lambdaSq, P -> x);
	mpz_sub(unmodded, temp1, Q -> x);
	mpz_mod(output -> x, unmodded, params -> p);

	mpz_sub(temp1, P -> x, output -> x);
	mpz_mul(temp2, temp1, lambda);
	mpz_sub(unmodded, temp2, P -> y);
	mpz_mod(output -> y, unmodded, params -> p);


	mpz_clear(topHalf);
	mpz_clear(lowerHalf);
	mpz_clear(lowerHalf_inv);
	mpz_clear(temp1);
	mpz_clear(temp2);
	mpz_clear(unmodded);
	mpz_clear(lambda);
	mpz_clear(lambdaSq);

	return output;
}


// Add two ECC points (P and Q) together and put the result in P.
void addEC_Point_PlusEqual(struct eccPoint *P, struct eccPoint *Q, struct eccParams *params)
{
	mpz_t topHalf, lowerHalf, lowerHalf_inv, temp1, temp2, unmodded, lambda, lambdaSq;

	mpz_init(topHalf);
	mpz_init(lowerHalf);
	mpz_init(lowerHalf_inv);
	mpz_init(temp1);
	mpz_init(temp2);
	mpz_init(unmodded);
	mpz_init(lambda);
	mpz_init(lambdaSq);


	mpz_sub(topHalf, Q -> y, P -> y);
	mpz_sub(lowerHalf, Q -> x, P -> x);
	mpz_invert(lowerHalf_inv, lowerHalf, params -> p);
	mpz_mul(lambda, topHalf, lowerHalf_inv);

	mpz_powm_ui(lambdaSq, lambda, 2, params -> p);
	mpz_sub(temp1, lambdaSq, P -> x);
	mpz_sub(unmodded, temp1, Q -> x);
	mpz_mod(topHalf, unmodded, params -> p);

	mpz_sub(temp1, P -> x, topHalf);
	mpz_mul(temp2, temp1, lambda);
	mpz_sub(unmodded, temp2, P -> y);

	mpz_set(P -> x, topHalf);
	mpz_mod(P -> y, unmodded, params -> p);


	mpz_clear(topHalf);
	mpz_clear(lowerHalf);
	mpz_clear(lowerHalf_inv);
	mpz_clear(temp1);
	mpz_clear(temp2);
	mpz_clear(unmodded);
	mpz_clear(lambda);
	mpz_clear(lambdaSq);
}


struct eccPoint *doublePoint(struct eccPoint *P, struct eccParams *params)
{
	if(P -> pointAtInf == 0x01)
	{
		// R = P
		return copyECC_Point(P);
	}
	else if(0 == mpz_cmp_ui(P -> y, 0))
	{
		// R = (@, @)
		return init_Identity_ECC_Point();	
	}
	else
	{
		// R = P * P
		return doubleGroupOp(P, params);
	}


	return NULL;
}



void doublePointInPlace(struct eccPoint *P, struct eccParams *params)
{
	if(P -> pointAtInf != 0x01)
	{
		if(0 != mpz_cmp_ui(P -> y, 0))
		{
			// R = P * P
			doubleGroupOpInPlace(P, params);
		}
		else
		{
			// R = (@, @)
			P -> pointAtInf = 1;	
		}
	}
}


struct eccPoint *groupOp(struct eccPoint *P, struct eccPoint *Q, struct eccParams *params)
{
	if(P -> pointAtInf == 0x01)
	{
		// R = Q
		return copyECC_Point(Q);
	}
	else if(Q -> pointAtInf == 0x01)
	{
		// R = P
		return copyECC_Point(P);
	}
	else if(0 != mpz_cmp(P -> x, Q -> x))
	{
		// R = P * Q
		return addEC_Point(P, Q, params);
	}
	else if(0 != mpz_cmp(P -> y, Q -> y) || 0 == mpz_cmp_ui(P -> y, 0))
	{
		// R = (@, @)
		return init_Identity_ECC_Point();
	}
	else
	{
		// This is effectively double
		// R = P * Q
		return doubleGroupOp(P, params);
	}

	return NULL;
}


// 
void groupOp_PlusEqual(struct eccPoint *P, struct eccPoint *Q, struct eccParams *params)
{
	if(Q -> pointAtInf != 0x01)
	{
		if(P -> pointAtInf == 0x01)
		{
			// R = Q
			mpz_set(P -> x, Q -> x);
			mpz_set(P -> y, Q -> y);
			P -> pointAtInf = 0x00;
		}
		else if(0 != mpz_cmp(P -> x, Q -> x))
		{
			// R = P * Q
			addEC_Point_PlusEqual(P, Q, params);
			P -> pointAtInf = 0x00;
		}
		else if(0 != mpz_cmp(P -> y, Q -> y) || 0 == mpz_cmp_ui(P -> y, 0))
		{
			// R = (@, @)
			P -> pointAtInf = 0x01;
		}
		else
		{
			// This is effectively double
			// R = P * Q
			doubleGroupOpInPlace(P, params);
			P -> pointAtInf = 0x00;
		}
	}
}


// Invert an Affine point.
struct eccPoint *invertPoint(struct eccPoint *P, struct eccParams *params)
{
	struct eccPoint *invP = initECC_Point();


	mpz_set(invP -> x, P -> x);
	mpz_sub(invP -> y, params -> p, P -> y);


	return invP;
}


// Perform Scalar multiplication using Double and add algorithm
struct eccPoint *doubleAndAdd_ScalarMul(mpz_t k, struct eccPoint *P, struct eccParams *params)
{
	struct eccPoint *Q = init_Identity_ECC_Point();
	struct eccPoint *tempP = copyECC_Point(P);
	int numBitsInK = mpz_sizeinbase(k, 2), i;

	for(i = 0; i < numBitsInK; i ++)
	{
		if(1 == mpz_tstbit(k, i))
		{
			groupOp_PlusEqual(Q, tempP, params);
		}

		doublePointInPlace(tempP, params);
	}

	clearECC_Point(tempP);

	return Q;
}


// Map a single MPZ to an ECC point. Note it might not be a VALID point on the curve.
struct eccPoint *mapMPZ_To_Point(mpz_t msg, struct eccParams *params)
{
	struct eccPoint *pointOutput = initECC_Point();
	mpz_t temp1, temp2, temp3;


	mpz_init(temp1);
	mpz_init(temp2);
	mpz_init(temp3);

	mpz_set(pointOutput -> x, msg);

	mpz_powm_ui(temp1, msg, 3, params -> p);
	mpz_mul(temp2, msg, params -> a);
	mpz_add(temp3, temp1, temp2);
	mpz_add(temp1, temp3, params -> b);
	// mpz_mod(temp2, temp1, params -> p);

	quadratic_residue(pointOutput -> y, temp1, params -> p);

	// z ^ ((p+1)/4) (mod p)
	mpz_clear(temp1);
	mpz_clear(temp2);
	mpz_clear(temp3);


	return pointOutput;
}



// Initialise a Params struct with the Brainpool 160 bit curve. (80 bit security)
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



// Initialise a Params struct with the Brainpool 256 bit curve. (128 bits security)
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


// Precomputes the power for Windowed scalar multi
struct eccPoint **preComputePoints(struct eccPoint *base, int windowSize, struct eccParams *params)
{
	struct eccPoint **output;
	int i;


	output = (struct eccPoint **) calloc(windowSize, sizeof(struct eccPoint*));
	output[1] = base;
	output[2] = doubleGroupOp(base, params);

	for(i = 3; i < windowSize; i ++)
	{
		output[i] = groupOp(output[i-1], base, params);
	}


	return output;
}


// Window Exponentation. Raise base to the power of exponent all modulo modularZ,
// store result in result.
struct eccPoint *windowedScalarPoint(mpz_t exponent, struct eccPoint *P, struct eccParams *params)
{
	//Get size of exponent in base 2.
	int i = mpz_sizeinbase(exponent, 2) - 1, u, j;
	int k = 4, twoPowerK = 16;

	struct eccPoint **preComputes;
	struct eccPoint *Q = init_Identity_ECC_Point();


	//Precompute the values for base to power of all possible windows. 
	preComputes = preComputePoints(P, twoPowerK, params);

	// We now run through the exponent from most significant bit.
	while (i >= 0)
	{
		//If the i-th bit (of exponent) is a zero, we just square the current result, move to next bit.
		u = 0;
		for(j = 0; j < k && i >= 0; j ++)
		{
			doublePointInPlace(Q, params);

			u = (u << 1) + mpz_tstbit(exponent, i);
			i --;
		}

		// If the window is not 0
		if(u)
		{			
			groupOp_PlusEqual(Q, preComputes[u], params);
		}
	}

	//Take out the rubbish
	for(i = 2; i < twoPowerK; i ++)
	{
		clearECC_Point(preComputes[i]);
	}
	free(preComputes);

	return Q;
}


// Take a base point g, compute 2^i * g for all i giving a valid exponent 
struct eccPoint **fixedBasePreComputes(struct eccPoint *base, struct eccParams *params)
{
	struct eccPoint **outputs;
	int i, bitSize = mpz_sizeinbase(params -> n, 2);


	outputs = (struct eccPoint **) calloc(bitSize + 1, sizeof(struct eccPoint *));

	outputs[0] = init_Identity_ECC_Point();
	outputs[1] = copyECC_Point(base);
	for(i = 2; i <= bitSize; i ++)
	{
		outputs[i] = doublePoint(outputs[i - 1], params);
	}


	return outputs;
}


// Perform a fixed based scalar multiplication.
struct eccPoint *fixedPointMultiplication(struct eccPoint **precomputes, mpz_t exponent, struct eccParams *params)
{
	struct eccPoint *output = init_Identity_ECC_Point();
	int i, bitSize = mpz_sizeinbase(params -> n, 2);
	mpz_t moddedExp;


	mpz_init(moddedExp);
	mpz_mod(moddedExp, exponent, params -> n);

	//For each bit in the exponent, add in the relevant pre-computed point.
	for(i = 1; i <= bitSize; i ++)
	{
		if( 1 == mpz_tstbit(moddedExp, i - 1) )
		{
			groupOp_PlusEqual(output, precomputes[i], params);
		}
	}


	return output;
}