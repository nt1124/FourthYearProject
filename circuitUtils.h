#ifndef CIRCUIT_UTILS
#define	CIRCUIT_UTILS

#include "crypto/cryptoUtil.h"
#include "gateOrWire.h"


typedef struct Circuit
{
	int circuitID;
	int numInputs;
	int numOutputs;
	int numGates;
	int *execOrder;
	struct gateOrWire **gates;
} Circuit;


typedef struct idAndValue
{
	int id;
	unsigned char value;

	struct idAndValue *next;
} idAndValue;


#include "fileUtils/fileUtilsFP.h"
#include "fileUtils/fileUtilsRTL.h"
#include "comms/sockets.h"


struct gateOrWire *processGateLine(char *line, struct gateOrWire **circuit);
struct gateOrWire **readInCircuit(char* filepath, int numGates);


#include "circuitUtils.c"


#endif

