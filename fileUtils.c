#include "fileUtils.h"



int *parseOutputTable(char* line, int tableSize, int *strIndex)
{
	int startIndex, tableIndex = 0, inputNumCount;
	int *tableToReturn = (int*) calloc(tableSize, sizeof(int));

	while( line[++ *strIndex] != '[' ) {}
	
	while( line[++ *strIndex] != ']' )
	{
		if( '1' == line[*strIndex] )
			tableToReturn[tableIndex ++] = 1;
		else if( '0' == line[*strIndex] )
			tableToReturn[tableIndex ++] = 0;
	}

	return tableToReturn;
}


int *parseInputTable(char* line, int tableSize, int *strIndex)
{
	int startIndex, tableIndex = -1, inputNumCount;
	int *tableToReturn = (int*) calloc(tableSize, sizeof(int));
	char *curCharStr = (char*) calloc(2, sizeof(char));

	while( line[++ *strIndex] != '[' ) {}

	while( line[++ *strIndex] != ']' )
	{
		if( ' ' == line[*strIndex] )
			tableIndex ++;
		else
		{
			tableToReturn[tableIndex] *= 10;
			curCharStr[0] = line[*strIndex];
			tableToReturn[tableIndex] += atoi(curCharStr);
		}
	}

	return tableToReturn;
}