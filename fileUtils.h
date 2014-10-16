#ifndef FILE_UTILS
#define	FILE_UTILS


struct outputEncRow *recursiveOutputTable(int *outputTable, struct gate *curGate);
int *parseOutputTable(char* line, int *strIndex, struct gate *curGate);
int *parseInputTable(char* line, int tableSize, int *strIndex);
int count_lines_of_file(char * filepath);

#include "fileUtils.c"

#endif