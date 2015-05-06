// Function send a single gate. Not used but included for completeness.
void sendGate(struct gateOrWire *inputGW, int writeSocket, int readSocket)
{
	unsigned char *buffer, *lengthBuffer = (unsigned char*) calloc(4, sizeof(unsigned char));
	int bufferLength, j;

	sendBoth(writeSocket, buffer, bufferLength);
	free(buffer);

}


// Send the full Circuit (gates, numGates, numOutputs etc.)
void sendCircuit(int writeSocket, int readSocket, struct Circuit *inputCircuit)
{
	unsigned char *bufferToSend, *circuitBuffer;
	int i, bufferLength, bufferOffset;
	int circuitLength = 0; 


	// Serialise the circuit.
	circuitBuffer = serialiseCircuit(inputCircuit, &circuitLength);

	// How big is buffer we're going to be sending, then calloc space.
	bufferLength = circuitLength + (7 * sizeof(int)) + (inputCircuit -> numGates * sizeof(int));
	bufferToSend = (unsigned char*) calloc(bufferLength, sizeof(unsigned char));


	// Copy Circuit parameters into the buffer we're going to send.
	memcpy(bufferToSend, &(inputCircuit -> numGates), sizeof(int));
	memcpy(bufferToSend + 1 * sizeof(int), &(inputCircuit -> numInputs), sizeof(int));
	memcpy(bufferToSend + 2 * sizeof(int), &(inputCircuit -> numOutputs), sizeof(int));
	memcpy(bufferToSend + 3 * sizeof(int), &(inputCircuit -> numInputsBuilder), sizeof(int));
	memcpy(bufferToSend + 4 * sizeof(int), &(inputCircuit -> numInputsExecutor), sizeof(int));
	memcpy(bufferToSend + 5 * sizeof(int), &(inputCircuit -> builderInputOffset), sizeof(int));
	memcpy(bufferToSend + 6 * sizeof(int), &(inputCircuit -> securityParam), sizeof(int));
	bufferOffset = 7 * sizeof(int);

	// Copy across the ExecOrder into the bufferToSend.
	memcpy(bufferToSend + bufferOffset, inputCircuit -> execOrder, inputCircuit -> numGates * sizeof(int));
	bufferOffset += (inputCircuit -> numGates * sizeof(int));

	// Copy across the Circuit into the bufferToSend.
	memcpy(bufferToSend + bufferOffset, circuitBuffer, circuitLength);

	// Send the buffer we've compiled.
	sendBoth(writeSocket, bufferToSend, bufferLength);

	// Housekeeping
	free(bufferToSend);
	free(circuitBuffer);
}


// Receive the order in which to execute the gates
int *receiveExecOrder(int writeSocket, int readSocket, int numGates)
{
	unsigned char *buffer;
	int *toReturn = (int*) calloc(numGates, sizeof(int));
	int bufferLength;

	bufferLength = receiveInt(readSocket);
	buffer = (unsigned char*) calloc(bufferLength, sizeof(unsigned char));
	receive(readSocket, buffer, bufferLength);
	memcpy(toReturn, buffer, bufferLength);

	return toReturn;
}


// Receive the actual circuit.
struct gateOrWire **receiveGatesOfCircuit(unsigned char *inputBuffer, int numGates)
{
	int i, j;
	struct gateOrWire **inputCircuit;

	inputCircuit = deserialiseCircuit(inputBuffer, numGates);

	return inputCircuit;
}


struct Circuit *receiveFullCircuit(int writeSocket, int readSocket)
{
	struct Circuit *inputCircuit = (struct Circuit*) calloc(1, sizeof(struct Circuit));
	unsigned char *receivedBuffer;
	int bufferLength, bufferOffset = 7 * sizeof(int);


	// bufferLength = receiveInt(readSocket);
	// receivedBuffer = (unsigned char*) calloc(bufferLength, sizeof(unsigned char));
	receivedBuffer = receiveBoth(readSocket, bufferLength);


	memcpy(&(inputCircuit -> numGates), receivedBuffer, sizeof(int));
	memcpy(&(inputCircuit -> numInputs), receivedBuffer + sizeof(int), sizeof(int));
	memcpy(&(inputCircuit -> numOutputs), receivedBuffer + 2 * sizeof(int), sizeof(int));
	memcpy(&(inputCircuit -> numInputsBuilder), receivedBuffer + 3 * sizeof(int), sizeof(int));
	memcpy(&(inputCircuit -> numInputsExecutor), receivedBuffer + 4 * sizeof(int), sizeof(int));
	memcpy(&(inputCircuit -> builderInputOffset), receivedBuffer + 5 * sizeof(int), sizeof(int));
	memcpy(&(inputCircuit -> securityParam), receivedBuffer + 6 * sizeof(int), sizeof(int));


	// Get the ExecOrder.
	inputCircuit -> execOrder = (int*) calloc(inputCircuit -> numGates, sizeof(int));
	memcpy(inputCircuit -> execOrder, receivedBuffer + bufferOffset, inputCircuit -> numGates * sizeof(int));
	bufferOffset += (inputCircuit -> numGates * sizeof(int));

	inputCircuit -> gates = receiveGatesOfCircuit(receivedBuffer + bufferOffset, inputCircuit -> numGates);

	free(receivedBuffer);

	return inputCircuit;
}
