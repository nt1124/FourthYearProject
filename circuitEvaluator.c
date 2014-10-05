#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>



typedef struct gate
{
	int numInputs;
	int *inputIDs;

	int outputTableSize;
	int *outputTable;
} gate;


typedef struct gateOrWire
{
	int G_ID;
	char typeTag;	// G = Gate, W = wire
	char wireValue;
	char outputFlag;

	struct gate *gate_data;
} gateOrWire;


void printGate(struct gate *input)
{
	int i;

	printf("NumInputs = %d\n", input -> numInputs);

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


void printGateOrWire(struct gateOrWire *input)
{
	if( 'W' == input -> typeTag )
	{
		printf("Wire ID  = %d\n", input -> G_ID);
	}
	else if( 'G' == input -> typeTag )
	{
		printf("Gate ID  = %d\n", input -> G_ID);
		printGate( input -> gate_data );
	}
	printf("Value = %d\n", input -> wireValue);
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


struct gate *processGate(char* line, int strIndex)
{
	struct gate* toReturn = calloc(1, sizeof(struct gate));
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

	return toReturn;
}


struct gateOrWire *processGateOrWire(char *line, int idNum, int *strIndex)
{
	struct gateOrWire *toReturn = calloc(1, sizeof(struct gateOrWire));

	toReturn -> G_ID = idNum;
	if( 'i' == line[*strIndex] )
	{
		toReturn -> typeTag = 'W';
		toReturn -> wireValue = rand() % 2;
	}
	else
	{
		toReturn -> typeTag = 'G';
		if('o' == line[*strIndex])
		{
			toReturn -> outputFlag = 1;
			while( line[*strIndex] != ' ' )
			{
				*strIndex = *strIndex + 1;
			}
		}
		toReturn -> gate_data = processGate(line, *strIndex);
	}

	return toReturn;
}


struct gateOrWire *processLine(char *line)
{
	int strIndex = 0, idNum;

	while( line[strIndex] != ' ' )
	{
		if( '/' == line[strIndex] )
			return NULL;
		strIndex ++;
	}

	char *idString = (char*) calloc(strIndex + 1, sizeof(char));
	strncpy(idString, line, strIndex);
	idNum = atoi(idString);

	while( line[strIndex] == ' ' )
	{
		if( '/' == line[strIndex] )
			return NULL;
		strIndex ++;
	}

	return processGateOrWire(line, idNum, &strIndex);
}


struct gateOrWire **readInCircuit(char* filepath, int numGates)
{
	int gateIndex = 0;
	struct gateOrWire *tempGateOrWire;
	struct gateOrWire **circuit = calloc(numGates, sizeof(struct gateOrWire));

	FILE *file = fopen ( filepath, "r" );
	if ( file != NULL )
	{
		char line [ 512 ]; /* or other suitable maximum line size */
		while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
		{
			tempGateOrWire = processLine(line);
			if( NULL != tempGateOrWire )
			{
				*(circuit + gateIndex) = tempGateOrWire;
				//printGateOrWire( tempGateOrWire );
				gateIndex ++;
			}
		}
		fclose ( file );
	}

	return circuit;
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


void runCircuit( struct gateOrWire **inputCircuit, int numGates )
{
	int i, j, tempIndex, numInputs;
	char outputTableIndex, tempValue;
	struct gate *currentGate;

	for(i = 0; i < numGates; i ++)
	{
		if( 'G' == inputCircuit[i] -> typeTag )
		{
			outputTableIndex = 0;
			currentGate = inputCircuit[i] -> gate_data;
			numInputs = currentGate -> numInputs;

			for(j = 0; j < numInputs; j ++)
			{
				tempIndex = currentGate -> inputIDs[numInputs - j - 1];
				outputTableIndex <<= 1;
				outputTableIndex += inputCircuit[tempIndex] -> wireValue;

				inputCircuit[i] -> wireValue = currentGate -> outputTable[outputTableIndex];
			}
		}

		if( 1 == inputCircuit[i] -> outputFlag )
		{
			printf("Id: %d has value %d.\n", inputCircuit[i] -> G_ID, inputCircuit[i] -> wireValue);
		}
	}
}


void readFormatFile( char *filepath)
{
    FILE *file = fopen ( filepath, "r" );
    int line_count = 0;

	if ( file != NULL )
	{
		char line [ 512 ]; /* or other suitable maximum line size */
		while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
		{
			if( 'B' == line[0] )
			{
				//We have a Bob thingy!	
			}
			else if( 'A' == line[0] )
			{
				// We have an Alice thingy!
			}
		}
		fclose ( file );
	}
}


int main(int argc, char *argv[])
{
	if( 2 != argc )
	{
		char *circuitFilepath = argv[1];
		char *formatFilepath = argv[2];

		int numGates = count_lines_of_file(circuitFilepath);
		printf("Num of Lines: %d\n", numGates);
		srand( time(NULL) );

		struct gateOrWire **inputCircuit = readInCircuit(circuitFilepath, numGates);
		runCircuit( inputCircuit, numGates );
	}
	else
	{
		printf("Circuit and Format file names required  %d\n", argc);
	}

	return 0;
}