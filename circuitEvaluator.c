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

	return 1;
}


int main(int argc, char *argv[])
{
	int i, sockfd;
	srand( time(NULL) );

	if( 1 != argc )
	{
		char *circuitFilepath = argv[1];

		int numGates = count_lines_of_file(circuitFilepath);

		struct gateOrWire **inputCircuit = readInCircuit(circuitFilepath, numGates);

		char tempAlice[] = "And.alice.input";
		char tempBob[] = "And.bob.input";
		readInputDetailsFile( tempAlice, inputCircuit );
		readInputDetailsFile( tempBob, inputCircuit );

		runCircuitExecutor( inputCircuit, numGates, sockfd );
		printAllOutput(inputCircuit, numGates);

		for(i = 0; i < numGates; i ++)
		{
			printf("+++++  Gate %02d  +++++\n", i);
			testSerialisation(inputCircuit[i]);
			printf("\n");
		}
	}
	else
	{
		printf("Circuit and Format file names required  %d\n", argc);
	}

	return 0;
}