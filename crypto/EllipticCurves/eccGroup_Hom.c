struct eccPoint_Hom *doublePoint_Hom(struct eccPoint_Hom *P, struct eccPoint_Hom *Q, struct eccParams_Hom *params)
{
	mpz_t temp1, temp2, temp3, temp4, w, wSq, wCb, pY_Sq;
	struct eccPoint_Hom *output = initECC_Point_Hom();

	mpz_inits(temp1, temp2, temp3, temp4, w, wSq, wCb, pY_Sq, NULL);


	// w = 3 * X1^2 + a * Z1^2
	mpz_mul(temp1, P -> x, P -> x);
	mpz_mul_ui(temp2, temp1, 3);
	mpz_mul(temp1, P -> z, P -> z);
	mpz_addmul(temp2, params -> a, temp1);
	
	mpz_mod(w, temp2, params -> p);
	mpz_powm_ui(wSq, w, 2, params -> p);
	mpz_powm_ui(wCb, w, 3, params -> p);
	mpz_powm_ui(pY_Sq, P -> y, 2, params -> p);


    // X3 = 2 * Y1 * Z1 * (w^2 - 8 * X1 * Y1^2 * Z1)
	mpz_mul_ui(temp1, P -> x, 8);
	mpz_mul(temp2, temp1, pY_Sq);
	mpz_submul(wSq, temp2, P -> z);
	mpz_mul(temp1, wSq, P -> z);
	mpz_mul(temp2, temp1, P -> y);
	mpz_mul_ui(temp3, temp2, 2);
	mpz_mod(output -> x, temp3, params -> p);

    // Y3 = 4 * Y1^2 * Z1 * (3 * w * X1 - 2 * Y1^2 * Z1) - w^3
	mpz_mul_ui(temp1, w, 3);
	mpz_mul(temp2, temp1, P -> x);
	mpz_mul_ui(temp3, pY_Sq, 2);
	mpz_submul(temp2, temp3, P -> z);
	mpz_mul(temp3, temp2, P -> z);
	mpz_mul(temp4, temp3, pY_Sq);
	mpz_mul_ui(temp1, temp4, 4);
	mpz_sub(temp2, temp1, wCb);
	mpz_mod(output -> y, temp2, params -> p);

    // Z3 = 8 * (Y1 * Z1)^3
	mpz_mul(temp1, P -> y, P -> z);
	mpz_powm_ui(temp2, temp1, 3, params -> p);
	mpz_mul_ui(temp3, temp2, 8);
	mpz_mod(output -> z, temp3, params -> p);


	return output;
}


struct eccPoint_Hom *addEC_Point_Hom(struct eccPoint_Hom *P, struct eccPoint_Hom *Q, mpz_t u, mpz_t v, struct eccParams_Hom *params)
{
	mpz_t temp1, temp2, temp3, temp4, temp5, uSq, vSq, uCb, vCb;
	struct eccPoint_Hom *output = initECC_Point_Hom();

	mpz_inits(temp1, temp2, temp3, temp4, temp5, uSq, vSq, uCb, vCb, NULL);


	mpz_mul(uSq, u, u);
	mpz_mul(vSq, v, v);
	mpz_mul(uCb, uSq, u);
	mpz_mul(vCb, vSq, v);

	// X3 = v * (Z2 * (Z1 * u^2 - 2 * X1 * v^2) - v^3)
	mpz_mul(temp1, P -> z, uSq);
	mpz_mul(temp2, P -> x, vSq);
	mpz_submul_ui(temp1, temp2, 2);
	mpz_mul(temp3, temp1, Q -> z);
	mpz_sub(temp4, temp3, vCb);
	mpz_mul(temp5, temp4, v);
	mpz_mod(output -> x, temp5, params -> p);


	// Y3 = Z2 * (3 * X1 * u * v^2 - Y1 * v^3 - Z1 * u^3) + u * v^3
	mpz_mul(temp1, u, vSq);
	mpz_mul(temp2, temp1, P -> x);
	mpz_mul_ui(temp3, temp2, 3);
	mpz_submul(temp3, P -> y, vCb);
	mpz_submul(temp3, P -> z, uCb);
	mpz_mul(temp4, temp3, Q -> z);
	mpz_addmul(temp4, u, vCb);
	mpz_mod(output -> y, temp4, params -> p);

	// Z3 = v^3 * Z1 * Z2
	mpz_mul(temp1, vSq, v);
	mpz_mul(temp2, temp1, P -> z);
	mpz_mul(temp3, temp2, Q -> z);
	mpz_mod(output -> z, temp3, params -> p);


	return output;
}


struct eccPoint_Hom *groupOp_Hom(struct eccPoint_Hom *P, struct eccPoint_Hom *Q, struct eccParams_Hom *params)
{
	struct eccPoint_Hom *output;
	mpz_t u, v, temp1, temp2, temp3;

	mpz_inits(u, v, temp1, temp2, temp3, NULL);

	// u = Y2 * Z1 - Y1 * Z2 and v = X2 * Z1 - X1 * Z2.
	mpz_mul(u, Q -> y, P -> z);
	mpz_submul(u, P -> y, Q -> z);

	mpz_mul(v, Q -> x, P -> z);
	mpz_submul(v, P -> x, Q -> z);


	if(P -> pointAtInf == 0x01)
	{
		// R = Q
		output = copyECC_Point_Hom(Q);
	}
	else if(Q -> pointAtInf == 0x01)
	{
		// R = P
		output = copyECC_Point_Hom(P);
	}
	else if(0 != mpz_cmp_ui(u, 0) && 0 == mpz_cmp_ui(v, 0))
	{
		// R = (@, @)
		output = initECC_Point_Hom();
		output -> pointAtInf = 1;
	}
	else if(0 != mpz_cmp_ui(u, 0) && 0 != mpz_cmp_ui(v, 0))
	{
		// R = P * Q
		output = addEC_Point_Hom(P, Q, u, v, params);
	}
	else
	{
		// This is effectively double
		// R = P * Q
		output = doublePoint_Hom(P, Q, params);
	}


	return output;
}



struct eccPoint_Hom *invertPoint_Hom(struct eccPoint_Hom *P, struct eccParams_Hom *params)
{
	struct eccPoint_Hom *invP = initECC_Point_Hom();


	mpz_set(invP -> x, P -> x);
	mpz_set(invP -> z, P -> z);
	mpz_sub(invP -> y, params -> p, P -> y);

	return invP;
}


struct eccPoint_Hom *doubleAndAdd_ScalarMul_Hom(mpz_t k, struct eccPoint_Hom *P, struct eccParams_Hom *params)
{
	struct eccPoint_Hom *Q = init_Identity_ECC_Point_Hom();
	struct eccPoint_Hom *tempP = copyECC_Point_Hom(P), *temp;
	int numBitsInK = mpz_sizeinbase(k, 2), i;

	for(i = 0; i < numBitsInK; i ++)
	{
		if(1 == mpz_tstbit(k, i))
		{
			temp = groupOp_Hom(Q, tempP, params);
			clearECC_Point_Hom(Q);
			Q = temp;
		}

		temp = groupOp_Hom(tempP, tempP, params);
		clearECC_Point_Hom(tempP);
		tempP = temp;
	}

	return Q;
}


struct eccPoint_Hom *mapMPZ_To_Point_Hom(mpz_t msg, struct eccParams_Hom *params)
{
	struct eccPoint_Hom *pointOutput = initECC_Point_Hom();
	mpz_t temp1, temp2, temp3;


	mpz_init(temp1);
	mpz_init(temp2);
	mpz_init(temp3);

	mpz_set(pointOutput -> x, msg);

	mpz_powm_ui(temp1, msg, 3, params -> p);
	mpz_mul(temp2, msg, params -> a);
	mpz_add(temp3, temp1, temp2);
	mpz_add(temp1, temp3, params -> b);
	mpz_mod(temp2, temp1, params -> p);

	quadratic_residue(pointOutput -> y, temp2, params -> p);

	mpz_set_ui(pointOutput -> z, 1);

	return pointOutput;
}


mpz_t *mapPoint_Hom_To_MPZ(struct eccPoint_Hom *inputPoint, struct eccParams_Hom *params)
{
	mpz_t *output = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_t unmodded, invZ;

	mpz_inits(*output, unmodded, invZ);


	mpz_invert(invZ, inputPoint -> z, params -> p);
	mpz_mul(unmodded, inputPoint -> x, invZ);
	mpz_mod(*output, unmodded, params -> p);


	return output;
}


struct eccPoint *convertToAffineRep(struct eccPoint_Hom *inputPoint, struct eccParams_Hom *params)
{
	struct eccPoint *pointOutput = initECC_Point();
	mpz_t unmodded, invZ;

	mpz_inits(unmodded, invZ);


	mpz_invert(invZ, inputPoint -> z, params -> p);

	mpz_mul(unmodded, inputPoint -> x, invZ);
	mpz_mod(pointOutput -> x, unmodded, params -> p);

	mpz_mul(unmodded, inputPoint -> y, invZ);
	mpz_mod(pointOutput -> y, unmodded, params -> p);


	return pointOutput;
}


struct eccParams_Hom *initBrainpool_160_Curve_Hom()
{
	const char *pStr = "E95E4A5F737059DC60DFC7AD95B3D8139515620F";
	const char *aStr = "340E7BE2A280EB74E2BE61BADA745D97E8F7C300";
	const char *bStr = "1E589A8595423412134FAA2DBDEC95C8D8675E58";
	const char *g_xStr = "BED5AF16EA3F6A4F62938C4631EB5AF7BDBCDBC3";
	const char *g_yStr = "1667CB477A1A8EC338F94741669C976316DA6321";
	const char *nStr = "E95E4A5F737059DC60DF5991D45029409E60FC09";

	struct eccParams_Hom *params = initECC_Params_Hom();


	gmp_sscanf(pStr, "%Zx", params -> p);
	gmp_sscanf(aStr, "%Zx", params -> a);
	gmp_sscanf(bStr, "%Zx", params -> b);
	gmp_sscanf(g_xStr, "%Zx", params -> g -> x);
	gmp_sscanf(g_yStr, "%Zx", params -> g -> y);
	mpz_set_ui(params -> g -> z, 1);
	gmp_sscanf(nStr, "%Zx", params -> n);


	return params;
}



struct eccParams_Hom *initBrainpool_256_Curve_Hom()
{
	const char *pStr = "A9FB57DBA1EEA9BC3E660A909D838D726E3BF623D52620282013481D1F6E5377";
	const char *aStr = "7D5A0975FC2C3057EEF67530417AFFE7FB8055C126DC5C6CE94A4B44F330B5D9";
	const char *bStr = "26DC5C6CE94A4B44F330B5D9BBD77CBF958416295CF7E1CE6BCCDC18FF8C07B6";
	const char *g_xStr = "8BD2AEB9CB7E57CB2C4B482FFC81B7AFB9DE27E1E3BD23C23A4453BD9ACE3262";
	const char *g_yStr = "547EF835C3DAC4FD97F8461A14611DC9C27745132DED8E545C1D54C72F046997";
	const char *nStr = "A9FB57DBA1EEA9BC3E660A909D838D718C397AA3B561A6F7901E0E82974856A7";

	struct eccParams_Hom *params = initECC_Params_Hom();


	gmp_sscanf(pStr, "%ZX", params -> p);
	gmp_sscanf(aStr, "%ZX", params -> a);
	gmp_sscanf(bStr, "%ZX", params -> b);
	gmp_sscanf(g_xStr, "%ZX", params -> g -> x);
	gmp_sscanf(g_yStr, "%ZX", params -> g -> y);
	mpz_set_ui(params -> g -> z, 1);
	gmp_sscanf(nStr, "%ZX", params -> n);


	return params;
}



// Precomputes the power for slidingWindowExp.
// This is mega ugly. We're just ignoring the even entries.
struct eccPoint_Hom **preComputePoints_Hom(struct eccPoint_Hom *base, struct eccParams_Hom *params)
{
	struct eccPoint_Hom **output;
	int i, windowSize = 16;
	mpz_t baseSquared;


	output = (struct eccPoint_Hom **) calloc(windowSize, sizeof(struct eccPoint_Hom*));

	output[0] = init_Identity_ECC_Point_Hom();
	output[1] = copyECC_Point_Hom(base);

	for(i = 2; i < windowSize; i ++)
	{
		output[i] = groupOp_Hom(output[i-1], base, params);
	}

	return output;
}


// Window Exponentation. Raise base to the power of exponent all modulo modularZ,
// store result in result.
struct eccPoint_Hom *windowedScalarPoint_Hom(mpz_t exponent, struct eccPoint_Hom *P, struct eccParams_Hom *params)
{
	//Get size of exponent in base 2.
	int i = mpz_sizeinbase(exponent, 2) - 1, h, s, u, j;
	int k = 4, twoPowerK = 16;

	struct eccPoint_Hom **preComputes;
	struct eccPoint_Hom *Q = init_Identity_ECC_Point_Hom();
	struct eccPoint_Hom *tempP = copyECC_Point_Hom(P), *temp;



	//Precompute the values for base to power of all possible windows. 
	preComputes = preComputePoints_Hom(P, params);

	while (i >= 0)
	{
		//If the i-th bit (of exponent) is a zero, we just square the current result, move to next bit.
		u = 0;
		for(j = 0; j < k && i >= 0; j ++)
		{
			temp = groupOp_Hom(Q, Q, params);
			clearECC_Point_Hom(Q);
			Q = temp;

			u = u << 1;
			u += mpz_tstbit(exponent, i);
			i --;
		}

		if(0 != u)
		{
			temp = groupOp_Hom(Q, preComputes[u], params);
			clearECC_Point_Hom(Q);
			Q = temp;
		}
	}

	//Take out the rubbish
	for(i = 0; i < twoPowerK; i += 2)
	{
		clearECC_Point_Hom(preComputes[i]);
	}

	return Q;
}
