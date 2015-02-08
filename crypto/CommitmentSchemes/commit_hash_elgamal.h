#ifndef HASH_ELGAMAL_COMMIT
#define HASH_ELGAMAL_COMMIT


#include "commit_elgamal.h"



struct elgamal_commit_key *deserialise_hash_elgamal_Kbox(unsigned char *xBytes, int *xBytesLen, unsigned char *inputBuffer, int *bufferOffset);


struct elgamal_commit_key *commit_hash_elgamal_C(struct commit_batch_params *params,
												unsigned char *toCommit, int toCommitLen,
												unsigned char *outputBuffer, int *bufferOffset,
												gmp_randstate_t state);
struct elgamal_commit_box *commit_hash_elgamal_R(struct commit_batch_params *params, unsigned char *inputBuffer, int *bufferOffset);


void decommit_hash_elgamal_C(struct commit_batch_params *params, struct elgamal_commit_key *k, unsigned char *outputBuffer, int *bufferOffset);
unsigned char *decommit_hash_elgamal_R(struct commit_batch_params *params, struct elgamal_commit_box *c,
										unsigned char *keyBuffer, int *keyOffset, int *outputLength);


struct commit_batch_params *setup_hash_elgamal_C(int writeSocket, int readSocket, int securityParam, gmp_randstate_t state);
struct commit_batch_params *setup_hash_elgamal_R(int writeSocket, int readSocket, int securityParam, gmp_randstate_t state);


#include "commit_hash_elgamal.c"


#endif