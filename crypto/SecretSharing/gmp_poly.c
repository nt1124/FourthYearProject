void printPoly(struct Fq_poly *polyToPrint)
{
	int i;

	for(i = 0; i <= polyToPrint -> degree; i ++)
	{
		if(0 != mpz_cmp_ui(polyToPrint -> coeffs[i], 0))
		{
			if(i == 0)
			{
				gmp_printf("%Zd", polyToPrint -> coeffs[i]);
			}
			else
			{
				gmp_printf("%Zd.X^%d", polyToPrint -> coeffs[i], i);
			}

			if(i < polyToPrint -> degree)
			{
				printf(" + ");
			}
		}
	}

	printf("\n");
}



void printPolyReverse(struct Fq_poly *polyToPrint)
{
	int i;

	for(i = polyToPrint -> degree; i >= 0; i --)
	{

		if(0 != mpz_cmp_ui(polyToPrint -> coeffs[i], 0))
		{
			if(i == 0)
			{
				gmp_printf("%Zd", polyToPrint -> coeffs[i]);
			}
			else
			{
				gmp_printf("%Zd.X^%d", polyToPrint -> coeffs[i], i);
			}

			if(i != 0)
			{
				printf(" + ");
			}
		}
	}

	printf("\n");
}


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


void freeFqPoly(struct Fq_poly *toFree)
{
	free(toFree -> coeffs);
	free(toFree);
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


struct Fq_poly *scalarMulti(struct Fq_poly *poly, mpz_t scalar, mpz_t q)
{
	struct Fq_poly *toReturn = initPolyWithDegree(poly -> degree);
	mpz_t temp;
	int i;

	mpz_init(temp);

	for(i = 0; i <= poly -> degree; i ++)
	{
		mpz_mul(temp, poly -> coeffs[i], scalar);
		mpz_mod(toReturn -> coeffs[i], temp, q);
	}

	return toReturn;
}


void scalarMultiInPlace(struct Fq_poly *poly, mpz_t scalar, mpz_t q)
{
	mpz_t temp;
	int i;

	mpz_init(temp);

	for(i = 0; i <= poly -> degree; i ++)
	{
		mpz_mul(temp, poly -> coeffs[i], scalar);
		mpz_mod(poly -> coeffs[i], temp, q);
	}

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
	tempArray = (mpz_t*) calloc(outputDegree + 1, sizeof(mpz_t));
	for(i = 0; i <= outputDegree; i ++)
	{
		mpz_init_set_ui(tempArray[i], 0);
	}

	for(i = 0; i <= x -> degree; i ++)
	{
		for(j = 0; j <= y -> degree; j ++)
		{
			mpz_mul(temp, x -> coeffs[i], y -> coeffs[j]);
			mpz_add(tempArray[i + j], tempArray[i + j], temp);
		}
	}


	while(0 == tempArray[outputDegree])
	{
		outputDegree --;
	}


	toReturn = setPolyWithArray(tempArray, q, outputDegree);

	return toReturn;
}


void setPolyCoeff(struct Fq_poly *poly, mpz_t newCoeff, int coeffIndexToSet, mpz_t q)
{
	mpz_mod(poly -> coeffs[coeffIndexToSet], newCoeff, q);
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
		mpz_mul(temp, *result, x);
		mpz_add(tempResult, temp, polyToEval -> coeffs[i]);
		mpz_mod(*result, tempResult, q);
	}

	return result;
}


void productOfMPZs(mpz_t output, mpz_t *inputArray, mpz_t q, int length)
{
	mpz_t unmoddedOutput;
	int i;


	mpz_init(output);
	mpz_init_set(unmoddedOutput, inputArray[0]);

	for(i = 1; i < length; i ++)
	{
		mpz_mul(unmoddedOutput, unmoddedOutput, inputArray[i]);
	}

	mpz_mod(output, unmoddedOutput, q);
}


int getHighestDegree(struct Fq_poly *inputPoly)
{
	int i = inputPoly -> degree;

	while(0 != mpz_cmp_ui(inputPoly -> coeffs[i], 0))
	{
		i --;
	}

	i --;

	return i;
}
