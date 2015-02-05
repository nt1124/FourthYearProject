#ifndef ELGAMAL_COMMIT
#define ELGAMAL_COMMIT


#include "../gmpUtils.c"
#include "../DDH_Primitive.h"
#include "../../comms/sockets.h"


typedef struct elgamal_commit_box
{
	mpz_t u;
	mpz_t v;
}  elgamal_commit_box;


typedef struct elgamal_commit_key
{
	mpz_t r;
	mpz_t x;
}  elgamal_commit_key;

typedef struct commit_batch_params
{
	struct DDH_Group *group;
	mpz_t h;
}  commit_batch_params;



struct commit_batch_params *init_commit_batch_params();
struct commit_batch_params *generate_commit_params(int securityParam, gmp_randstate_t state);

int getSerialSizeElgamal(struct DDH_Group *group, int numToSerialise);
struct elgamal_commit_box *init_commit_box();
struct elgamal_commit_key *init_commit_key();

void serialise_elgamal_Cbox(struct elgamal_commit_box *c, unsigned char *outputBuffer, int *bufferOffset);
struct elgamal_commit_box * deserialise_elgamal_Cbox(unsigned char *inputBuffer, int *bufferOffset);

struct elgamal_commit_key *single_commit_elgamal_C(struct commit_batch_params *params, mpz_t valueToCommit,	unsigned char *outputBuffer, int *bufferOffset,	gmp_randstate_t state);
struct elgamal_commit_box *single_commit_elgamal_R(struct commit_batch_params *params, unsigned char *inputBuffer, int *bufferOffset);

void single_commit_elgamal_C(struct commit_batch_params *params, struct elgamal_commit_key *k, unsigned char *outputBuffer, int *bufferOffset);
unsigned char single_decommit_elgamal_R(struct commit_batch_params *params, struct elgamal_commit_box *c, unsigned char *keyBuffer, int *keyOffset);


int send_commit_batch_params(int writeSocket, int readSocket, struct commit_batch_params *params);
struct commit_batch_params *receive_commit_batch_params(int writeSocket, int readSocket);

struct commit_batch_params *setup_elgamal_C(int writeSocket, int readSocket, int securityParam, gmp_randstate_t state);
struct commit_batch_params *setup_elgamal_R(int writeSocket, int readSocket);



#include "commit_elgamal.c"


#endif