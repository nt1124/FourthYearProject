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

	int *outputTable;
	int outputValue;
} gate;


struct wire* constructWire(int idNum, char inputOrOutput)
{
	struct wire *newInput = calloc(1, sizeof(struct wire));

	newInput -> W_ID = idNum;
	newInput -> wireValue = 0;
	newInput -> inputOrOutput = inputOrOutput;

	return newInput;
}


struct gate *processGate(char* line, int id, int strIndex)
{
	struct gate* toReturn = calloc(1, sizeof(struct gate));
	int temp;

	while( line[strIndex] != ' ' )
	{ strIndex ++; }
	while( line[strIndex] != ' ' )
	{ strIndex ++; }
	temp = strIndex;

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

	if( 'i' == line[strIndex] || 'o' == line[strIndex] )
	{
		struct wire *newInput = constructWire(idNum, line[strIndex]);
	}
	else
	{

	}
}


void readFileLines(char* filepath)
{
	FILE *file = fopen ( filepath, "r" );
	if ( file != NULL )
	{
		char line [ 128 ]; /* or other suitable maximum line size */
		while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
		{
		   fputs ( line, stdout ); /* write the line */
		}
		fclose ( file );
	}
	else
	{
		return;
	}
}


int main()
{
	readFileLines("Billionaires.txt.Opt.circuit");


	return 0;	
}