#ifndef CIRCUIT_UTILS
#define	CIRCUIT_UTILS

#include "crypto/cryptoUtil.h"
#include "gateOrWire.h"


struct gateOrWire *processGateLine(char *line, struct gateOrWire **circuit);
struct gateOrWire **readInCircuit(char* filepath, int numGates);


typedef struct Circuit
{
	int numGates;
	struct gateOrWire **gates;
} Circuit;


#include "circuitUtils.c"


#endif

