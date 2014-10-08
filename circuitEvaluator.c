#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "circuitUtils.c"
#include "cryptoUtil.c"
#include "galois.c"

void testGalois()
{
	int a = 132;
	int b = 43;
	int w = 8;

	int output = galois_single_multiply(a, b, w);
	printf("%d\n", output);
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

		char tempAlice[] = "And.alice.input";
		char tempBob[] = "And.bob.input";
		readInputDetailsFile( tempAlice, inputCircuit );
		readInputDetailsFile( tempBob, inputCircuit );

		runCircuit( inputCircuit, numGates );
		// printAllOutput(inputCircuit, numGates);

		aesTestGCrypt();
		// testGalois();
		// aesTest();
    	// sha256Digest("123456781234567812345678", 24);
	}
	else
	{
		printf("Circuit and Format file names required  %d\n", argc);
	}

	return 0;
}