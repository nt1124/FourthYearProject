#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// #include "circuitUtils.c"
#include "Circuit.cpp"
#include "formatUtils.c"



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

		Circuit inputCircuit = Circuit(circuitFilepath, numGates);
		struct formatItem **inputFormats = readFormatFile(formatFilepath, numInputOutputs);

		char tempAlice[] = "And.alice.input";
		char tempBob[] = "And.bob.input";

		readInputDetailsFile( tempAlice, inputCircuit, 5 );
		readInputDetailsFile( tempBob, inputCircuit, 3 );

		inputCircuit.getInputKeys();
		inputCircuit.runCircuit();
		inputCircuit.printAllOutput();
	}
	else
	{
		printf("Circuit and Format file names required  %d\n", argc);
	}

	return 0;
}