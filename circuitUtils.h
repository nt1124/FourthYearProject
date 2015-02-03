#ifndef CIRCUIT_UTILS
#define	CIRCUIT_UTILS

#include "crypto/cryptoUtil.h"
#include "gateOrWire.h"
#include "timingUtils.c"




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
#include "circuitExecutor.c"
#include "circuitBuilder.c"


#endif

