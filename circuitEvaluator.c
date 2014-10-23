#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "circuitUtils.h"



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
		printAllOutput(inputCircuit, numGates);

		// testAES();
    	// sha256Digest("123456781234567812345678", 24);
    	// testElgamal();
    	// testRSA();
    	// testByteConvert();
	}
	else
	{
		printf("Circuit and Format file names required  %d\n", argc);
	}

	return 0;
}