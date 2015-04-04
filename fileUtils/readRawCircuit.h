#ifndef RAW_CIRCUIT_READER
#define	RAW_CIRCUIT_READER




typedef struct RawGate
{
	int G_ID;

	unsigned char numInputs;		// Number of input wires to gate.
	int *inputIDs;					// ID of the input wires to gate.

	unsigned char wireMask;
	unsigned char gateType;			// XOR, AND, INV, NXOR or MegaAND.
	unsigned short outputTableSize;	// 2 ^ numInputs.
	int *rawOutputTable;			// Raw output table for gate, unencrypted/unpermutated.

	unsigned char outputValue;
} RawGate;


typedef struct RawCircuit
{
	int numInputs;
	int numInputs_P1;
	int numInputs_P2;
	int numOutputs;
	int numGates;

	int *execOrder;
	struct RawGate **gates;
} RawCircuit;


#include "../circuits/gateOrWire.h"
#include "readRawCircuit.c"

#endif