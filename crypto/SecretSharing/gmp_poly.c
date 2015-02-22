#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


// Index of coeff equals its power. x[0] = x ^ 0
typedef struct Fq_poly
{
	int degree;
	mpz_t *coeffs;
} Fq_poly;



struct Fq_poly *initPolyWithDegree(int degree)
{
	struct Fq_poly *toReturn = (struct Fq_poly*) calloc(1, sizeof(struct Fq_poly));
	int i;


	toReturn -> degree = degree;
	toReturn -> coeffs = (mpz_t*) calloc(degree + 1, sizeof(mpz_t));

	for(i = 0; i <= degree; i ++)
	{
		mpz_init(toReturn -> coeffs[i]);
	}

	return toReturn;
}


struct Fq_poly *setPolyWithArray(mpz_t *inputArray, mpz_t q, int degree)
{
	struct Fq_poly *toReturn = initPolyWithDegree(degree);
	int i;


	for(i = 0; i <= degree; i ++)
	{
		mpz_mod(toReturn -> coeffs[i], inputArray[i], q);
	}

	return toReturn;
}


struct Fq_poly *addPolys_Internal(struct Fq_poly *x, struct Fq_poly *y, mpz_t q)
{
	struct Fq_poly *toReturn;
	mpz_t temp;
	int i, maxDegree, minDegree;


	mpz_init(temp);
	maxDegree = x -> degree;
	minDegree = y -> degree;
	toReturn = initPolyWithDegree(maxDegree);

	for(i = 0; i <= minDegree; i ++)
	{
		mpz_add(temp, x -> coeffs[i], y -> coeffs[i]);
		mpz_mod(toReturn -> coeffs[i], temp, q);
	}

	for(; i <= maxDegree; i ++)
	{
		mpz_mod(toReturn -> coeffs[i], x -> coeffs[i], q);
	}

	return toReturn;
}


struct Fq_poly *addPolys(struct Fq_poly *x, struct Fq_poly *y, mpz_t q)
{
	struct Fq_poly *toReturn;

	if(x -> degree >= y -> degree)
	{
		toReturn = addPolys_Internal(x, y, q);
	}
	else
	{
		toReturn = addPolys_Internal(y, x, q);
	}

	return toReturn;
}


struct Fq_poly *mulPolys(struct Fq_poly *x, struct Fq_poly *y, mpz_t q)
{
	struct Fq_poly *toReturn;
	mpz_t *tempArray, temp;
	int i, j, outputDegree = x -> degree + y -> degree;


	mpz_init(temp);
	for(i = 0; i < outputDegree; i ++)
	{
		mpz_init(tempArray[i]);
	}

	for(i = 0; i <= x -> degree; i ++)
	{
		for(j = 0; j <= y -> degree; j ++)
		{
			mpz_mul(temp, x -> coeffs[i], y -> coeffs[j]);
			mpz_add(tempArray[i + j], temp, q);
		}
	}

	while(0 == tempArray[outputDegree])
	{
		outputDegree --;
	}

	toReturn = setPolyWithArray(tempArray, q, outputDegree);

	return toReturn;
}


mpz_t *evalutePoly(struct Fq_poly *polyToEval, mpz_t x, mpz_t q)
{
	mpz_t *result = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_t temp, tempResult;
	int i;


	mpz_init(temp);
	mpz_init_set_ui(*result, 0);
	mpz_init(tempResult);

	for(i = polyToEval -> degree; i >= 0; i--)
	{
		// res = res * x + coeffs[i];
		mpz_mul(temp, *result, x);
		mpz_add(tempResult, temp, polyToEval -> coeffs[i]);
		mpz_mod(*result, tempResult, q);
	}

	return result;
}



void testPolys()
{
	struct Fq_poly *polyToEval;
	mpz_t *inputArray, q, x, *result;
	int degree = 1, i;
	int length = degree + 1;

	inputArray = (mpz_t*) calloc(length, sizeof(mpz_t));


	mpz_init_set_ui(q, 1024);
	mpz_init_set_ui(x, 3);
	for(i = 0; i < length; i ++)
	{
		mpz_init_set_ui(inputArray[i], 3);
	}


	polyToEval = setPolyWithArray(inputArray, q, degree);
	result = evalutePoly(polyToEval, x, q);

	gmp_printf("%Zd\n", *result);
}