#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "circuitUtils.h"


int compilationOfTests()
{
	// testAES();
	// testElgamal();
	// testRSA();
	// testByteConvert();
	// testSerialisation(inputCircuit[16]);

	return 1;
}


int main(int argc, char *argv[])
{
	int i;
	srand( time(NULL) );

	if( 1 != argc )
	{
		char *circuitFilepath = argv[1];

		int numGates = count_lines_of_file(circuitFilepath);

		struct gateOrWire **inputCircuit = readInCircuit(circuitFilepath, numGates);
		// struct formatItem **inputFormats = readFormatFile(formatFilepath, numInputOutputs);

		char tempAlice[] = "And.alice.input";
		char tempBob[] = "And.bob.input";
		readInputDetailsFile( tempAlice, inputCircuit );
		readInputDetailsFile( tempBob, inputCircuit );

		runCircuit( inputCircuit, numGates );
		printAllOutput(inputCircuit, numGates);
	}
	else
	{
		printf("Circuit and Format file names required  %d\n", argc);
	}

	return 0;
}