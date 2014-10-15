#ifndef __MODMUL_H
#define __MODMUL_H

#include  <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <string.h>

#include    <gmp.h>

//#include  <fcntl.h>
//#include <unistd.h>
#include <time.h>

#include <openssl/evp.h>

//#define MAX(a,b) (((a) > (b)) ? (a) : (b)) // this isn't safe as evaluates the maximum argument twice

#define MAX(a,b) \
  ({ typeof (a) _a = (a); \
    typeof (b) _b = (b); \
    _a > _b ? _a : _b; })

void testMontMul();
void testMontgomeryFunctions();
void testRecode();
void testExtractBits();
void testExponentiation();
void testRand();
void writeRandomBytesToFile();

#endif
