#include "fileUtils.h"



struct outputEncRow *recursiveOutputTable(int *outputTable, struct gate *curGate)
{
	struct outputEncRow *toReturn = calloc(1, sizeof(struct outputEncRow));
	struct outputEncRow *tempRow;
	int i, j, tempBit;

	for(i = 0; i < curGate -> outputTableSize; i ++)
	{
		tempRow = toReturn;
		for(j = 0; j < curGate -> numInputs; j ++)
		{
			tempBit = (i >> j) & 1;
			if(NULL == tempRow -> zeroOrOne[tempBit])
			{
				tempRow -> zeroOrOne[tempBit] = calloc(1, sizeof(struct outputEncRow));
			}

			tempRow = tempRow -> zeroOrOne[tempBit];
		}

		tempRow -> outputValue = *(outputTable + i);
	}

	return toReturn;
}


int *parseOutputTable(char* line, int *strIndex, struct gate *curGate)
{
	int startIndex, tableIndex = 0, inputNumCount;
	int *tableToParse = (int*) calloc(curGate -> outputTableSize, sizeof(int));

	while( line[++ *strIndex] != '[' ) {}
	
	while( line[++ *strIndex] != ']' )
	{
		if( '1' == line[*strIndex] )
			tableToParse[tableIndex ++] = 1;
		else if( '0' == line[*strIndex] )
			tableToParse[tableIndex ++] = 0;
	}

	return tableToParse;
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