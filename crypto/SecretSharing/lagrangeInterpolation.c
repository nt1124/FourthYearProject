

struct Fq_poly **getLagrangeFactorsDivProduct(mpz_t divFactorProduct,int n, int *delta_i, unsigned int i, mpz_t q)
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
	}


	return output;
}


void testPolys()
{
	struct Fq_poly **lagrangePolys;
	struct Fq_poly *output;
	mpz_t *codewords, q, *temp;
	int degree = 6, length = degree + 1, i;
	int delta_i[7] = {1, 2, 3, 4, 5, 6, 13};


	codewords = (mpz_t*) calloc(length, sizeof(mpz_t));


	for(i = 0; i < length; i ++)
	{
		mpz_init(codewords[i]);
	}
	mpz_init_set_ui(q, 101);

	mpz_set_ui(codewords[0], 44);
	mpz_set_ui(codewords[1], 2);
	mpz_set_ui(codewords[2], 96);
	mpz_set_ui(codewords[3], 23);
	mpz_set_ui(codewords[4], 86);
	mpz_set_ui(codewords[5], 83);
	mpz_set_ui(codewords[6], 32);


	output = getPolyFromCodewords(codewords, delta_i, 3, q);
	printPolyReverse(output);
}