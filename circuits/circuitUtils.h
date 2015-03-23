#ifndef CIRCUIT_UTILS
#define	CIRCUIT_UTILS


const int stat_SecParam = 4;

typedef struct idAndValue
{
	int id;
	unsigned char value;

	struct idAndValue *next;
} idAndValue;


#include "timingUtils.c"
#include "../crypto/cryptoUtil.h"
#include "gateOrWire.h"



#include "../fileUtils/inputFileUtils.h"
#include "../fileUtils/readRawCircuit.h"
#include "../fileUtils/fileUtilsRTL.h"
#include "../fileUtils/fileUtils_FromRaw.h"

#include "../comms/sockets.h"


struct gateOrWire *processGateLine(char *line, struct gateOrWire **circuit);
struct gateOrWire **readInCircuit(char* filepath, int numGates);


#include "circuitUtils.c"
#include "circuitExecutor.c"
#include "circuitBuilder.c"
#include "circuitCheckerUtils.c"

#include "Naive_SH/SH_Circuits.c"
#include "LP_2010/LP_S2C_CnC_OT.h"
#include "L_2013/L_S2C_CnC_OT.h"



#endif

