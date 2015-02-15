#ifndef CIRCUIT_UTILS
#define	CIRCUIT_UTILS

#include "timingUtils.c"
#include "../crypto/cryptoUtil.h"
#include "gateOrWire.h"




typedef struct idAndValue
{
	int id;
	unsigned char value;

	struct idAndValue *next;
} idAndValue;


#include "../fileUtils/fileUtilsFP.h"
#include "../fileUtils/fileUtilsRTL.h"
#include "../fileUtils/fileUtilsRTL_CnC.h"
#include "../comms/sockets.h"


struct gateOrWire *processGateLine(char *line, struct gateOrWire **circuit);
struct gateOrWire **readInCircuit(char* filepath, int numGates);


#include "circuitUtils.c"
#include "circuitExecutor.c"
#include "circuitBuilder.c"

#include "Naive_SH/SH_Circuits.c"
#include "LP_2014/LP_S2C_CnC_OT.c"


#endif

