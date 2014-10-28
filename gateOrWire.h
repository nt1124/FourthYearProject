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
	unsigned char numInputs;
	int *inputIDs;

	unsigned short outputTableSize;
	unsigned char **encOutputTable;
	int *rawOutputTable;
} gate;

typedef struct wire
{
	unsigned char wireMask;
	unsigned char wirePerm;
	unsigned char wirePermedValue;
	unsigned char *wireOutputKey;
	struct bitsGarbleKeys *outputGarbleKeys;
} wire;

typedef struct gateOrWire
{
	int G_ID;

	struct wire *outputWire;
	struct gate *gatePayload;
} gateOrWire;


void printGate(struct gate *input);
void printGateOrWire(struct gateOrWire *input);

struct gate *processGate(char* line, int strIndex, struct gateOrWire **circuit, struct gateOrWire *curGate);
struct gateOrWire *processGateOrWire(char *line, int idNum, int *strIndex, struct gateOrWire **circuit);

#include "fileUtils.h"
#include "gateOrWire.c"
#include <math.h>




#endif