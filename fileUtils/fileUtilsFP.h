#ifndef FILE_UTILS_FP
#define	FILE_UTILS_FP


// unsigned char **recursiveOutputTable(struct gate *curGate);

int *parseOutputTable(char* line, int *strIndex, struct gate *curGate);
int *parseInputTable(char* line, int tableSize, int *strIndex);
int count_lines_of_file(char * filepath);
struct gate *processGateFP(char* line, int strIndex, struct gateOrWire **circuit, struct gateOrWire *curGate);
struct gateOrWire *processGateOrWireFP(char *line, int idNum, int *strIndex, struct gateOrWire **circuit);
struct gateOrWire *processGateLineFP(char *line, struct gateOrWire **circuit);
struct gateOrWire **readInCircuitFP(char* filepath, int *numGates);

#include "../gateOrWire.h"
#include "fileUtilsFP.c"

#endif