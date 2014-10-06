#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "circuitUtils.c"



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
	}
}


int main(int argc, char *argv[])
{
	int i;
	srand( time(NULL) );

	if( 2 != argc )
	{
		char *circuitFilepath = argv[1];
		char *formatFilepath = argv[2];

		int numGates = count_lines_of_file(circuitFilepath);
		int numInputOutputs = count_lines_of_file(formatFilepath);

		struct gateOrWire **inputCircuit = readInCircuit(circuitFilepath, numGates);
		struct formatItem **inputFormats = readFormatFile(formatFilepath, numInputOutputs);

		readInputDetailsFile( "And.alice.input", inputCircuit );
		readInputDetailsFile( "And.bob.input", inputCircuit );

		runCircuit( inputCircuit, numGates );
		printAllOutput(inputCircuit, numGates);

		printf("Size of long^2: %d\n", sizeof(unsigned long long));
	}
	else
	{
		printf("Circuit and Format file names required  %d\n", argc);
	}

	return 0;
}