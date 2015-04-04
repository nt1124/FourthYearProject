#ifndef GATE_OR_WIRE
#define GATE_OR_WIRE


typedef struct bitsGarbleKeys
{
	unsigned char *key0; // Ronseal.
	unsigned char *key1; // Ronseal
} bitsGarbleKeys;


typedef struct gate
{
	unsigned char numInputs;		// Number of input wires to gate.
	int *inputIDs;					// ID of the input wires to gate.

	unsigned char gateType;			// XOR, AND, INV. (ALSO INV_XOR)
	unsigned short outputTableSize;	// 2 ^ numInputs.
	unsigned char **encOutputTable; // The encrypted and permutated output table.
	int *rawOutputTable;			// Raw output table for gate, unencrypted/unpermutated.

} gate;


typedef struct wire
{
	unsigned char wireOwner; 				 // 0xFF indicates owned by circuit builder. 0x00 else.
	unsigned char wireMask;					 // 0x01 = input wire; 0x02 = output wire.
	unsigned char wirePerm;					 // Only the first bit matters. Not efficient but...
	unsigned char wirePermedValue;			 // The bit value of the wire permutated.
	unsigned char *wireOutputKey;			 // 16 bytes, the key to be used in keyList.
	struct bitsGarbleKeys *outputGarbleKeys; // The two possible keys to be used for wireOutputKey.
											 // Only one of which is known to executor.
} wire;


typedef struct gateOrWire
{
	int G_ID;					// Unique ID.

	struct wire *outputWire;	// Represents the output of the gate/input wire
	struct gate *gatePayload;	// Represents gate behaviours, is NULL for input wires
} gateOrWire;


typedef struct Circuit
{
	int circuitID;
	int numInputs;
	int numInputsBuilder;
	int numInputsExecutor;
	int numOutputs;
	int numGates;
	int builderInputOffset;

	unsigned int seed;
	int securityParam;

	unsigned char checkFlag;

	int *execOrder;
	struct gateOrWire **gates;
} Circuit;


// Missing so many functions here!
void printGateOrWire(struct gateOrWire *inputGW);

unsigned char **createOutputTable(struct gate *curGate);
void encWholeOutTable(struct gateOrWire *curGate, struct gateOrWire **circuit);

void decryptGate(struct gateOrWire *curGate, struct gateOrWire **inputCircuit);
void freeXOR_Gate(struct gateOrWire *curGate, struct gateOrWire **inputCircuit);
void evaulateGate(struct gateOrWire *curGate, struct gateOrWire **inputCircuit);

struct bitsGarbleKeys *generateGarbleKeyPair(unsigned char perm);
struct bitsGarbleKeys *genFreeXORPairInput(unsigned char perm, unsigned char *R);
struct bitsGarbleKeys *genFreeXORPair(struct gateOrWire *curGate, unsigned char *R, struct gateOrWire **circuit);
unsigned char getPermutation();

void freeGateOrWire(struct gateOrWire *inputGW);



#include "serialisationUtils.c"
#include "gateOrWire.c"
#include <math.h>


#endif