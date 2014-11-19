#ifndef CIRCUIT_UTILS
#define	CIRCUIT_UTILS

#include "crypto/cryptoUtil.h"
#include "gateOrWire.h"
#include "fileUtils/fileUtilsFP.h"
#include "fileUtils/fileUtilsRTL.h"
#include "comms/sockets.h"


struct gateOrWire *processGateLine(char *line, struct gateOrWire **circuit);
struct gateOrWire **readInCircuit(char* filepath, int numGates);


#include "circuitUtils.c"


#endif

