#ifndef __AES_H
#define __AES_H

// #include "Networking/data.h"
// REMEMBER TO USE -maes

typedef unsigned int  uint;
// typedef unsigned char unsigned char;

#include "randUtils.c"
#include <string.h>
#include <wmmintrin.h>
#include "aes.cpp"

#define AES_BLK_SIZE 16

/************* C Version *************/
// Key Schedule
void aes_schedule( int nb, int nr, unsigned char* k, uint* RK );

inline void aes_schedule( uint* RK, unsigned char* K )
{ aes_schedule(4, 10, K, RK); }
inline void aes_128_schedule( uint* RK, unsigned char* K )
{ aes_schedule(4, 10, K, RK); }
inline void aes_192_schedule( uint* RK, unsigned char* K )
{ aes_schedule(6, 12, K, RK); }
inline void aes_256_schedule( uint* RK, unsigned char* K )
{ aes_schedule(8, 14, K, RK); }


// Encryption Function 
void aes_128_encrypt( unsigned char* C, unsigned char* M, uint* RK );
void aes_192_encrypt( unsigned char* C, unsigned char* M, uint* RK );
void aes_256_encrypt( unsigned char* C, unsigned char* M, uint* RK );

inline void aes_encrypt( unsigned char* C, unsigned char* M, uint* RK )
{ aes_128_encrypt(C,M,RK ); }


/*********** M-Code Version ***********/


// Check can support this
int Check_CPU_support_AES();
// Key Schedule 
void aes_128_schedule( unsigned char* key, const unsigned char* userkey );
void aes_192_schedule( unsigned char* key, const unsigned char* userkey );
void aes_256_schedule( unsigned char* key, const unsigned char* userkey );

inline void aes_schedule( unsigned char* key, const unsigned char* userkey )
{ aes_128_schedule(key,userkey); }


// Encryption Function 
void aes_128_encrypt( unsigned char* C, const unsigned char* M,const unsigned char* RK );
void aes_192_encrypt( unsigned char* C, const unsigned char* M,const unsigned char* RK );
void aes_256_encrypt( unsigned char* C, const unsigned char* M,const unsigned char* RK );

inline void aes_encrypt( unsigned char* C, const unsigned char* M,const unsigned char* RK )
{ aes_128_encrypt(C,M,RK); }


#endif

