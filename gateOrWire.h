#ifndef GATE_OR_WIRE
#define GATE_OR_WIRE

typedef struct outputEncRow
{
	unsigned char outputValue;
	unsigned char *outputEncValue;
} outputEncRow;


typedef struct bitsGarbleKeys
{
	unsigned char *key0;
	unsigned char *key1;
} bitsGarbleKeys;


typedef struct gate
{
	char numInputs;
	int *inputIDs;

	short outputTableSize;
	struct outputEncRow **outputTreeEnc;
	unsigned char tablePermutation;

	struct bitsGarbleKeys **inputKeySet;
} gate;


typedef struct gateOrWire
{
	int G_ID;
	char wireValue;
	unsigned char *wireEncValue;
	char outputFlag;

	struct bitsGarbleKeys *outputGarbleKeys;
	struct gate *gate_data;
} gateOrWire;

/*


typedef struct gate
{
	char numInputs;
	int *inputIDs;

	short outputTableSize;
	unsigned char **encOutputTable;
} gate;

typedef struct wire
{
	int inputIDs;
	unsigned char wireOwnerOutput;	//LSB denoted owner, MSB denotes output or not
	unsigned char wirePermutation;
	unsigned char wireValue;
	unsigned char *wireEncValue;
	struct bitsGarbleKeys *outputGarbleKeys;
} wire;

typedef struct gateOrWire
{
	int G_ID;

	struct bitsGarbleKeys *outputGarbleKeys;
} gateOrWire;


*/

void printGate(struct gate *input);
void printGateOrWire(struct gateOrWire *input);

struct gate *processGate(char* line, int strIndex, struct gateOrWire **circuit, struct gateOrWire *curGate);
struct gateOrWire *processGateOrWire(char *line, int idNum, int *strIndex, struct gateOrWire **circuit);

#include "fileUtils.h"
#include "gateOrWire.c"
#include <math.h>




#endif