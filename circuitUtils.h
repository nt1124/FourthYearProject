#ifndef CIRCUIT_UTILS
#define	CIRCUIT_UTILS

#include "cryptoUtil.c"


typedef struct outputEncRow
{
	struct outputEncRow *zeroOrOne[2];

	unsigned char outputValue;
	unsigned char *outputEncValue;
}outputEncRow;


typedef struct bitsGarbleKeys
{
	unsigned char key0[16];
	unsigned char key1[16];
}bitsGarbleKeys;


typedef struct gate
{
	int numInputs;
	int *inputIDs;

	int outputTableSize;
	struct outputEncRow *outputTreeEnc;

	struct bitsGarbleKeys **inputKeySet;
} gate;


typedef struct gateOrWire
{
	int G_ID;
	char typeTag;		// G = Gate, W = wire
	char wireValue;
	char outputFlag;
	unsigned char wireEnc[16];

	struct bitsGarbleKeys *outputGarbleKeys;
	struct gate *gate_data;
} gateOrWire;


void printGate(struct gate *input);
void printGateOrWire(struct gateOrWire *input);

struct gate *processGate(char* line, int strIndex, struct gateOrWire **circuit, struct gateOrWire *curGate);
struct gateOrWire *processGateOrWire(char *line, int idNum, int *strIndex, struct gateOrWire **circuit);
struct gateOrWire *processGateLine(char *line, struct gateOrWire **circuit);
struct gateOrWire **readInCircuit(char* filepath, int numGates);

int count_lines_of_file(char * filepath);

#endif

