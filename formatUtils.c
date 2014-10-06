#include "formatUtils.h"
#include "fileUtils.c"


void printFormatItem(struct formatItem *input)
{	
	int i;

	if( 'i' == input -> inputOrOutput )
		printf(" Input  =  ");
	else if( 'o' == input -> inputOrOutput )
		printf("Output  =  ");

	printf("Owner = %c\n", input -> owner);

	printf("[ %d", input -> idList[0]);
	for( i = 1; i < input -> bitLength; i ++ )
	{
		printf(", %d", input -> idList[i]);
	}
	printf(" ]\n\n");
}


int countWires(char* line, int strIndex)
{
	int wireCount = 0;
	int localIndex = strIndex;

	while( '[' != line[localIndex ++] ) {	}

	while( ']' != line[localIndex++] )
	{
		if( ' ' == line[localIndex] )
		{
			wireCount ++;
		}
	}
	
	return wireCount;
}


struct formatItem *readFormatLine(char *line)
{
	struct formatItem *toReturn = (struct formatItem*) calloc(1, sizeof(struct formatItem));
	int strIndex = 0;

	toReturn -> owner = line[0];
	
	while( ' ' != line[strIndex ++] ) {	}
	toReturn -> inputOrOutput = line[strIndex];

	toReturn -> bitLength = countWires(line, strIndex);
	toReturn -> idList = parseInputTable(line, toReturn -> bitLength, &strIndex);

	return toReturn;
}


struct formatItem **readFormatFile( char *filepath, int numInputOutputs)
{
	FILE *file = fopen ( filepath, "r" );
	int line_count = 0, formatIndex = 0;

	struct formatItem **formatList = (struct formatItem**) calloc(numInputOutputs, sizeof(struct formatList*));
	struct formatItem *formatItem;

	if ( file != NULL )
	{
		char line [ 512 ]; /* or other suitable maximum line size */
		while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
		{
			formatItem = readFormatLine(line);
			if( NULL != formatItem )
			{
				*(formatList + formatIndex) = formatItem;
				formatIndex ++;
			}
		}
		fclose ( file );
	}

	return formatList;
}
