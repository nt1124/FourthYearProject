/*
void runCircuitBuilder( struct Circuit *inputCircuit, int writeSocket, int readSocket)
{
	struct wire *tempWire;
	int i, numInputs = 0, tempSize;

	unsigned char *receivedBuffer, *outputBuffer;
	int receivedOffset = 0, outputOffset = 0;

	struct timespec timestamp_0 = timestamp(), timestamp_1;
	clock_t c_0, c_1;
	c_0 = clock();

	gmp_randstate_t *state = seedRandGen();
	struct decParams *params = senderCRS_Syn_Dec(writeSocket, readSocket, 1024, *state);

	builder_side_OT(writeSocket, readSocket, params, inputCircuit, state);

	c_1 = clock();
	timestamp_1 = timestamp();

	printTiming(&timestamp_0, &timestamp_1, c_0, c_1, "OT - Sender");
}
*/