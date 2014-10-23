#ifndef __AES_H
#define __AES_H

// #include "Networking/data.h"

typedef unsigned int  uint;

#define AES_BLK_SIZE 16

/************* C Version *************/
// Key Schedule
void aes_schedule( int nb, int nr, octet* k, uint* RK );

inline void aes_schedule( uint* RK, octet* K )
{ aes_schedule(4, 10, K, RK); }
inline void aes_128_schedule( uint* RK, octet* K )
{ aes_schedule(4, 10, K, RK); }
inline void aes_192_schedule( uint* RK, octet* K )
{ aes_schedule(6, 12, K, RK); }
inline void aes_256_schedule( uint* RK, octet* K )
{ aes_schedule(8, 14, K, RK); }

// Encryption Function 
void aes_128_encrypt( octet* C, octet* M, uint* RK );
void aes_192_encrypt( octet* C, octet* M, uint* RK );
void aes_256_encrypt( octet* C, octet* M, uint* RK );

inline void aes_encrypt( octet* C, octet* M, uint* RK )
{ aes_128_encrypt(C,M,RK ); }


/*********** M-Code Version ***********/
// Check can support this
int Check_CPU_support_AES();
// Key Schedule 
void aes_128_schedule( octet* key, const octet* userkey );
void aes_192_schedule( octet* key, const octet* userkey );
void aes_256_schedule( octet* key, const octet* userkey );

inline void aes_schedule( octet* key, const octet* userkey )
{ aes_128_schedule(key,userkey); }


// Encryption Function 
void aes_128_encrypt( octet* C, const octet* M,const octet* RK );
void aes_192_encrypt( octet* C, const octet* M,const octet* RK );
void aes_256_encrypt( octet* C, const octet* M,const octet* RK );

inline void aes_encrypt( octet* C, const octet* M,const octet* RK )
{ aes_128_encrypt(C,M,RK); }


#endif

