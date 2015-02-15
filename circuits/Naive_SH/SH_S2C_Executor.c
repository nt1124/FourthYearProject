// Run the circuit, executor style.
/*
void runCircuitExec( struct Circuit *inputCircuit, int writeSocket, int readSocket, char *filepath )
{
	unsigned char *tempBuffer;
	int i, gateID, outputLength = 0, j, nLength;
	struct timespec timestamp_0, timestamp_1;
	clock_t c_0, c_1;

	readInputDetailsFileExec(writeSocket, readSocket, filepath, inputCircuit);

	timestamp_0 = timestamp();
	c_0 = clock();


	for(i = 0; i < inputCircuit -> numGates; i ++)
	{
		gateID = inputCircuit -> execOrder[i];

		if( NULL != inputCircuit -> gates[gateID] -> gatePayload )
		{
			evaulateGate(inputCircuit -> gates[gateID], inputCircuit -> gates);
		}
	}

	c_1 = clock();
	timestamp_1 = timestamp();


	printTiming(&timestamp_0, &timestamp_1, c_0, c_1, "Circuit Evalutation");
}
*/