#ifndef GMP_POLY
#define GMP_POLY


#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


// Index of coeff equals its power. x[0] = x ^ 0
typedef struct Fq_poly
{
	int degree;
	mpz_t *coeffs;
} Fq_poly;


#include "gmp_poly.c"

#endif