#ifndef FILE_UTILS_RTL
#define	FILE_UTILS_RTL



struct gate *processGateRTL(char* line, int strIndex, struct gateOrWire **circuit, struct gateOrWire *curGate);
struct gateOrWire *processGateOrWireRTL(char *line, int idNum, int *strIndex, struct gateOrWire **circuit);
struct gateOrWire *processGateLineRTL(char *line, struct gateOrWire **circuit);
struct gateOrWire **readInCircuitRTL(char* filepath, int *numGates);

#include "../gateOrWire.h"
#include "fileUtilsRTL.c"

#endif