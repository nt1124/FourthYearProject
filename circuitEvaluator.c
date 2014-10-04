#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef struct wire
{
	int W_ID;
	int wireValue;
	char inputOrOutput;
} wire;

typedef struct gate
{
	int G_ID;
	int numInputs;
	int *inputIDs;

	int outputTableSize;
	int *outputTable;
	int outputValue;
} gate;


typedef struct gateOrWire
{
	union
	{
		struct gate gate_data;
		struct wire wire_data;
	};
} gateOrWire;


void printGate(struct gate *input)
{
	int i;

	printf("Table ID  = %d\n", input -> G_ID);
	printf("numInputs = %d\n", input -> numInputs);

	printf("[ %d", *((input -> inputIDs)) );
	for(i = 1; i < input -> numInputs; i ++)
	{
		printf(", %d", *((input -> inputIDs) + i) );
	}
	printf(" ]\n");

	printf("[ %d", *((input -> outputTable)) );
	for(i = 1; i < input -> outputTableSize; i ++)
	{
		printf(", %d", *((input -> outputTable) + i) );
	}
	printf(" ]\n\n");
}

struct wire* constructWire(int idNum, char inputOrOutput)
{
	struct wire *newInput = calloc(1, sizeof(struct wire));

	newInput -> W_ID = idNum;
	newInput -> wireValue = 0;
	newInput -> inputOrOutput = inputOrOutput;

	return newInput;
}


int *parseOutputTable(char* line, int tableSize, int *strIndex)
{
	int startIndex, tableIndex = 0, inputNumCount;
	int *tableToReturn = calloc(tableSize, sizeof(int));

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
	int *tableToReturn = calloc(tableSize, sizeof(int));
	char *curCharStr = calloc(2, sizeof(char));

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


struct gate *processGate(char* line, int id, int strIndex)
{
	struct gate* toReturn = calloc(1, sizeof(struct gate));
	toReturn -> G_ID= id;
	int tempIndex, outputTableSize = 1;

	while( line[++ strIndex] != ' ' ) {}
	while( line[++ strIndex] != ' ' ) {}
	tempIndex = ++ strIndex;
	while( line[strIndex] != ' ' ) { strIndex ++;}

	char *tempString = calloc((strIndex - tempIndex + 1), sizeof(char));
	strncpy(tempString, line + tempIndex, (strIndex - tempIndex));
	toReturn -> numInputs = atoi(tempString);

	for(tempIndex = 0; tempIndex < toReturn -> numInputs; tempIndex ++)
		outputTableSize *= 2;
	
	toReturn -> outputTableSize = outputTableSize;
	toReturn -> outputTable = parseOutputTable(line, outputTableSize, &strIndex);
	toReturn -> inputIDs = parseInputTable(line, toReturn -> numInputs, &strIndex);

	//printGate(toReturn);

	return toReturn;
}


void processLine(char *line)
{
	int strIndex = 0, idNum;

	while( line[strIndex] != ' ' )
	{ strIndex ++; }

	char *idString = (char*) calloc(strIndex + 1, sizeof(char));
	strncpy(idString, line, strIndex);
	idNum = atoi(idString);

	while( line[strIndex] == ' ' )
	{ strIndex ++; }

	if( 'i' == line[strIndex] )
	{
		struct wire *newInput = constructWire(idNum, line[strIndex]);
	}
	else
	{
		if('o' == line[strIndex])
		{
			while( line[strIndex] == ' ' )
			{ strIndex ++; }
		}
		processGate(line, idNum, strIndex);
	}
}

int count_lines_of_file(char * filepath) {
    FILE *file = fopen ( filepath, "r" );
    int line_count = 0;

	if ( file != NULL )
	{
		char line [ 512 ]; /* or other suitable maximum line size */
		while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
		{
			if( 48 <= line[0] && line[0] < 58)
			{
				line_count ++;	
			}
		}
		fclose ( file );
		return line_count;
	}
    return -1;
}

void readInCircuit(char* filepath)
{
	FILE *file = fopen ( filepath, "r" );
	if ( file != NULL )
	{
		char line [ 512 ]; /* or other suitable maximum line size */
		while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
		{
			processLine(line);
		   //fputs ( line, stdout ); /* write the line */
		}
		fclose ( file );
	}
}


int main()
{
	printf("Num of Lines: %ld\n", count_lines_of_file("Billionaires.txt.Opt.circuit"));
	readInCircuit("Billionaires.txt.Opt.circuit");

	return 0;	
}