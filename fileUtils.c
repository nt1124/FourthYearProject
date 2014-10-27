
unsigned char **recursiveOutputTable(int *outputTable, struct gate *curGate)
{
	unsigned char **toReturn = (unsigned char**) calloc(curGate -> outputTableSize, sizeof(unsigned char*));
	int i;

	for(i = 0; i < curGate -> outputTableSize; i ++)
	{
		toReturn[i] = (unsigned char*) calloc(1, sizeof(unsigned char));
		toReturn[i][16] = *(outputTable + i);
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


int count_lines_of_file(char * filepath)
{
    FILE *file = fopen ( filepath, "r" );
    int line_count = 0;

	if ( file != NULL )
	{
		char line [ 512 ]; /* or other suitable maximum line size */
		while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
		{
			if( (48 <= line[0] && line[0] < 58) ||
				( 'A' == line[0] || 'B' == line[0] ) )
			{
				line_count ++;	
			}
		}
		fclose ( file );
		return line_count;
	}

    return -1;
}
