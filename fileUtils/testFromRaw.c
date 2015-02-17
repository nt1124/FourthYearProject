void testRawCircuits(char *circuitFilepath, char *inputFilepath_B, char *inputFilepath_E)
{
	struct RawCircuit *inputCircuit = readInCircuit_Raw(circuitFilepath);
	struct idAndValue *startOfInputChain, *start;


	startOfInputChain = readInputDetailsFile_Alt(inputFilepath_B);
	start = startOfInputChain;
	setRawCircuitsInputs_Hardcode(start, inputCircuit -> gates);
	free_idAndValueChain(startOfInputChain);


	startOfInputChain = readInputDetailsFile_Alt(inputFilepath_E);
	start = startOfInputChain;
	setRawCircuitsInputs_Hardcode(start, inputCircuit -> gates);
	free_idAndValueChain(startOfInputChain);


	evaluateRawCircuit(inputCircuit);

	printf("\n");
	printOutputHexString_Raw(inputCircuit);
	testAES_FromRandom();
}


void testGarbledFromRawCircuits(char *circuitFilepath, char *inputFilepath_B, char *inputFilepath_E)
{
	struct RawCircuit *rawInputCircuit = readInCircuit_Raw(circuitFilepath);
	struct Circuit *garbledCircuit;
	struct idAndValue *startOfInputChain, *start;


	garbledCircuit = readInCircuit_FromRaw(rawInputCircuit);


	startOfInputChain = readInputDetailsFile_Alt(inputFilepath_B);
	start = startOfInputChain;
	setCircuitsInputs_Hardcode(start, garbledCircuit, 0xFF);
	free_idAndValueChain(startOfInputChain);


	startOfInputChain = readInputDetailsFile_Alt(inputFilepath_E);
	start = startOfInputChain;
	setCircuitsInputs_Hardcode(start, garbledCircuit, 0xFF);
	free_idAndValueChain(startOfInputChain);

	runCircuitLocal(garbledCircuit);

	printOutputHexString(garbledCircuit);
	//evaluateRawCircuit(rawInputCircuit);

}