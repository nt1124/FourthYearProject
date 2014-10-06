#include "circuitUtils.h"
#include "formatUtils.c"


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


void printAllOutput(struct gateOrWire **inputCircuit, int numGates)
{
	int i;

	for(i = 0; i < numGates; i ++)
	{
		if( 1 == inputCircuit[i] -> outputFlag )
		{
			printf("Gate %d = %d\n", inputCircuit[i] -> G_ID, inputCircuit[i] -> wireValue);
		}
	}
}



struct gate *processGate(char* line, int strIndex)
{
	struct gate* toReturn = (struct gate*) calloc(1, sizeof(struct gate));
	int tempIndex, outputTableSize = 1;

	while( line[++ strIndex] != ' ' ) {}
	while( line[++ strIndex] != ' ' ) {}
	tempIndex = ++ strIndex;
	while( line[strIndex] != ' ' ) { strIndex ++;}

	char *tempString = (char*) calloc((strIndex - tempIndex + 1), sizeof(char));
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
	struct gateOrWire *toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));

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


struct gateOrWire *processGateLine(char *line)
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
	struct gateOrWire **circuit = (struct gateOrWire**) calloc(numGates, sizeof(struct gateOrWire*));

	FILE *file = fopen ( filepath, "r" );
	if ( file != NULL )
	{
		char line [ 512 ]; /* or other suitable maximum line size */
		while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
		{
			tempGateOrWire = processGateLine(line);
			if( NULL != tempGateOrWire )
			{
				*(circuit + gateIndex) = tempGateOrWire;
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


void readInputLines(char *line, struct gateOrWire **inputCircuit)
{
	int strIndex = 0, gateID = 0, wireValue;
	char *curCharStr = (char*) calloc( 2, sizeof(char) );

	while( ' ' != line[strIndex++] ){}

	while( ' ' != line[strIndex] )
	{
		gateID *= 10;
		curCharStr[0] = line[strIndex];
		gateID += atoi(curCharStr);
		strIndex ++;
	}
	strIndex ++;

	if( '1' == line[strIndex] )
		wireValue = 1;
	else if( '0' == line[strIndex] )
		wireValue = 0;

	inputCircuit[gateID] -> wireValue = wireValue;
}


void readInputDetailsFile(char *filepath, struct gateOrWire **inputCircuit)
{
	FILE *file = fopen ( filepath, "r" );
	if ( file != NULL )
	{
		char line [ 512 ]; /* or other suitable maximum line size */
		while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
		{
			readInputLines(line, inputCircuit);
		}
		fclose ( file );
	}
}