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

// struct eccPoint **gPreComputes;


#include "../fileUtils/inputFileUtils.h"
#include "../fileUtils/readRawCircuit.h"
#include "../fileUtils/fileUtilsRTL.h"
#include "../fileUtils/fileUtils_FromRaw.h"
#include "../fileUtils/fileUtils_FromRaw_HKE_2013.h"


#include "../comms/sockets.h"


struct gateOrWire *processGateLine(char *line, struct gateOrWire **circuit);
struct gateOrWire **readInCircuit(char* filepath, int numGates);


#include "circuitUtils.c"
#include "circuitCommunication.c"
#include "circuitCorrectnessChecks.c"


#include "LP_2010/LP_S2C_CnC_OT.h"
#include "L_2013/L_S2C_CnC_OT.h"
#include "HKE_2013/HKE_2013.h"

#include "L_2013_HKE/L_2013_HKE.h"



#endif

