#ifndef FILE_UTILS
#define	FILE_UTILS


unsigned char **recursiveOutputTable(struct gate *curGate);
int *parseOutputTable(char* line, int *strIndex, struct gate *curGate);
int *parseInputTable(char* line, int tableSize, int *strIndex);
int count_lines_of_file(char * filepath);

#include "fileUtils.c"

#endif