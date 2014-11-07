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

	unsigned short outputTableSize;	// 2 ^ numInputs.
	unsigned char **encOutputTable; // The encrypted and permutated output table.
	int *rawOutputTable;			// Raw output table for gate, unencrypted/unpermutated.
									// Only used temporarily by Builder. Try to get rid of in future.
} gate;


typedef struct wire
{
	unsigned char wireOwner; 				 // 0xFF indicates owned by circuit builder. 0x00 else.
	unsigned char wireMask;					 // 0xF0 = input wire; 0x0F = output wire.
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


// Missing so many functions here!
void freeGateOrWire(struct gateOrWire *inputGW);

//#include "fileUtils.h"
#include "serialisationUtils.c"
#include "gateOrWire.c"
#include <math.h>




#endif