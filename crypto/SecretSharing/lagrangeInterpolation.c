

struct Fq_poly **getLagrangeFactorsDivProduct(mpz_t divFactorProduct, int n, int *delta_i, unsigned int i, mpz_t q)
{
	struct Fq_poly **factors = (struct Fq_poly **) calloc(n, sizeof(struct Fq_poly *));
	mpz_t *inputArray = (mpz_t *) calloc(2, sizeof(mpz_t));
	mpz_t *divFactors = (mpz_t *) calloc(n, sizeof(mpz_t));
	mpz_t temp, x_i;
	unsigned int j;


	mpz_init(temp);
	mpz_init(divFactorProduct);
	mpz_init(inputArray[0]);
	mpz_init_set_ui(inputArray[1], 1);
	mpz_init_set_ui(x_i, delta_i[i-1]);

	for(j = 1; j <= n; j ++)
	{

		mpz_init(divFactors[j-1]);

		if(i != j)
		{
			mpz_sub_ui(temp, x_i, delta_i[j-1]);
			mpz_mod(divFactors[j-1], temp, q);
			mpz_sub_ui(inputArray[0], q, delta_i[j-1]);

			factors[j-1] = setPolyWithArray(inputArray, q, 1);
		}
		else
		{
			mpz_init_set_ui(divFactors[j-1], 1);
			mpz_init_set_ui(inputArray[0], 1);
			factors[j-1] = setPolyWithArray(inputArray, q, 0);
		}
	}


	productOfMPZs(divFactorProduct, divFactors, q, n);

	return factors;
}


struct Fq_poly *generateLagrangePoly(int n, int *delta_i, unsigned int i, mpz_t q)
{
	struct Fq_poly **factors, **intermediates, *output;
	mpz_t divFactor, divFactorInv;
	int j;


	intermediates = (struct Fq_poly **) calloc(n - 1, sizeof(struct Fq_poly*));
	mpz_init(divFactorInv);


	factors = getLagrangeFactorsDivProduct(divFactor, n, delta_i, i, q);

	if(n > 1)
	{
		intermediates[0] = mulPolys(factors[0], factors[1], q);

		for(j = 1; j < n - 1; j ++)
		{
			intermediates[j] = mulPolys(intermediates[j - 1], factors[j + 1], q);
		}
	}
	else
	{
		intermediates[0] = factors[0];
	}

	mpz_invert(divFactorInv, divFactor, q);

	output = scalarMulti(intermediates[n - 2], divFactorInv, q);


	return output;
}


struct Fq_poly **generateAllLagrangePolys(int length, int *delta_i, mpz_t q)
{
	struct Fq_poly **lagrangePolys;
	int i;


	lagrangePolys = (struct Fq_poly **) calloc(length, sizeof(struct Fq_poly *));

	for(i = 1; i <= length; i ++)
	{
		lagrangePolys[i-1] = generateLagrangePoly(length, delta_i, i, q);
	}

	return lagrangePolys;
}




// Length = t?
struct Fq_poly *getPolyFromCodewords(mpz_t *codewords, int *delta_i, int length, mpz_t q)
{
	struct Fq_poly **lagrangePolys, *output;
	int i;


	lagrangePolys = generateAllLagrangePolys(length, delta_i, q);
	output = initPolyWithDegree(length); 


	for(i = 1; i <= length; i ++)
	{
		scalarMultiInPlace(lagrangePolys[i-1], codewords[i-1], q);
		output = addPolys(lagrangePolys[i-1], output, q);

		// gmp_printf("%Zd\n", codewords[i-1]);
	}


	return output;
}



mpz_t *completePartialSecretSharing(int *iAlready, mpz_t *c_iAlready, mpz_t C, mpz_t q, int stat_secParam)
{
	struct Fq_poly *secretPoly;
	mpz_t *fixedCodeword = (mpz_t*) calloc((stat_secParam / 2) + 1, sizeof(mpz_t));
	mpz_t *finalCodewords = (mpz_t*) calloc(stat_secParam, sizeof(mpz_t));
	mpz_t tempPoint, *tempOutput;

	int *delta_i = (int*) calloc((stat_secParam / 2) + 1, sizeof(int));
	unsigned char *temp = (unsigned char*) calloc(stat_secParam, sizeof(unsigned char));
	unsigned int i;


	mpz_init(tempPoint);

	for(i = 0; i < stat_secParam / 2; i ++)
	{
		temp[iAlready[i] - 1] = 0x01;
	
		mpz_init(fixedCodeword[i]);
		mpz_set(fixedCodeword[i], c_iAlready[i]);

		mpz_init(finalCodewords[iAlready[i] - 1]);
		mpz_set(finalCodewords[iAlready[i] - 1], c_iAlready[i]);
	}

	mpz_init(fixedCodeword[stat_secParam / 2]);
	mpz_set(fixedCodeword[stat_secParam / 2], C);

	memcpy(delta_i, iAlready, sizeof(int) * (stat_secParam / 2));
	delta_i[stat_secParam / 2] = stat_secParam + 1;


	secretPoly = getPolyFromCodewords(fixedCodeword, delta_i, stat_secParam / 2 + 1, q);

	for(i = 0; i < stat_secParam; i ++)
	{
		if(temp[i] == 0x00)
		{
			mpz_set_ui(tempPoint, i + 1);
			tempOutput = evalutePoly(secretPoly, tempPoint, q);
			mpz_init_set(finalCodewords[i], *tempOutput);
		}
	}

	mpz_set_ui(tempPoint, stat_secParam + 1);
	tempOutput = evalutePoly(secretPoly, tempPoint, q);


	return finalCodewords;
}


int testSecretScheme(struct Fq_poly *polyToTest, mpz_t secret, mpz_t q, unsigned int threshold, unsigned int numShares)
{
	mpz_t secret_index, *polyTestSecret;
	int degreeOfPoly = getHighestDegree(polyToTest);

	mpz_t *temp;
	int i;


	mpz_init_set_ui(secret_index, numShares + 1);

	polyTestSecret = evalutePoly(polyToTest, secret_index, q);


	for(i = 0; i < numShares; i ++)
	{
		mpz_set_ui(secret_index, i + 1);
		temp = evalutePoly(polyToTest, secret_index, q);
	}

	if(threshold == degreeOfPoly + 1 &&	0 == mpz_cmp(*polyTestSecret, secret))
	{
		return 0;
	}

	return 1;
}



void testPolys()
{
	struct Fq_poly **lagrangePolys;
	struct Fq_poly *output;
	mpz_t *codewords, q, *temp;
	int degree = 6, length = degree + 1, i;
	int delta_i[4] = {1, 2, 3, 4};
	int finalDecision;


	codewords = (mpz_t*) calloc(length, sizeof(mpz_t));


	for(i = 0; i < length; i ++)
	{
		mpz_init(codewords[i]);
	}
	mpz_init_set_ui(q, 101);

	mpz_set_ui(codewords[0], 44);
	mpz_set_ui(codewords[1], 2);
	mpz_set_ui(codewords[2], 96);
	mpz_set_ui(codewords[3], 86);


	temp = completePartialSecretSharing(delta_i, codewords, codewords[3], q, 4);

	output = getPolyFromCodewords(temp, delta_i, 4, q);
}