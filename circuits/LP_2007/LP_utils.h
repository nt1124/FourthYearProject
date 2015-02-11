#ifndef LP_UTILS
#define LP_UTILS


typedef struct commit_W_Boxes
{
	int length;

	struct elgamal_commit_box *b_Box;
	struct elgamal_commit_box **k_Boxes;

	struct elgamal_commit_box *b_dot_Box;
	struct elgamal_commit_box **k_dot_Boxes;
} commit_W_Boxes;

typedef struct commit_Circuit_Boxes
{
	int length;
	struct commit_W_Boxes **rows;
} commit_Circuit_Boxes;

typedef struct commit_Wire_Boxes
{
	int length;
	struct commit_Circuit_Boxes **columns;
} commit_Wire_Boxes;



typedef struct commit_W_Keys
{
	int length;
	unsigned char b;

	struct elgamal_commit_key *b_Key;
	struct elgamal_commit_key **k_Keys;

	struct elgamal_commit_key *b_dot_Key;
	struct elgamal_commit_key **k_dot_Keys;
} commit_W_Keys;

typedef struct commit_Circuit_Keys
{
	int length;
	struct commit_W_Keys **rows;
} commit_Circuit_Keys;

typedef struct commit_Wire_Keys
{
	int length;
	struct commit_Circuit_Keys **columns;
} commit_Wire_Keys;



#include "LP_commitment.c"
#include "LP_Malicious.c"


#endif