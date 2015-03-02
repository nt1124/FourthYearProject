void readInputLinesBuilder(char *line, struct gateOrWire **inputCircuit)
{
	int strIndex = 0, gateID = 0;
	struct wire *outputWire;

	while( ' ' != line[strIndex++] ){}

	while( ' ' != line[strIndex] )
	{
		gateID *= 10;
		gateID += line[strIndex] - 48;
		strIndex ++;
	}
	strIndex ++;

	outputWire = inputCircuit[gateID] -> outputWire;

	outputWire -> wireOwner = 0xFF;
	if( '1' == line[strIndex] )
	{
		outputWire -> wirePermedValue = outputWire -> outputGarbleKeys -> key1[16];
		memcpy(outputWire -> wireOutputKey, outputWire -> outputGarbleKeys -> key1, 16);
	}
	else if( '0' == line[strIndex] )
	{
		outputWire -> wirePermedValue = outputWire -> outputGarbleKeys -> key0[16];
		memcpy(outputWire -> wireOutputKey, outputWire -> outputGarbleKeys -> key0, 16);
	}
}


void readInputDetailsFileBuilder(char *filepath, struct gateOrWire **inputCircuit)
{
	FILE *file = fopen ( filepath, "r" );
	if ( file != NULL )
	{
		char line [ 512 ];
		while ( fgets ( line, sizeof line, file ) != NULL )
		{
			readInputLinesBuilder(line, inputCircuit);
		}
		fclose ( file );
	}
}




void builder_side_OT_ECC(int writeSocket, int readSocket, struct decParams_ECC *params, struct Circuit *inputCircuit, gmp_randstate_t *state)
{
	struct u_v_Pair_ECC **c_i_array;
	struct otKeyPair_ECC **keyPairs;
	struct wire *tempWire;

	unsigned char *receivedBuffer, *outputBuffer;
	int receivedOffset = 0, outputOffset = 0, tempSize, numInputs = 0;
	int i, j, inputGatesOffset = 0;


	receivedOffset = receiveInt(readSocket);
	receivedBuffer = (unsigned char*) calloc(receivedOffset, sizeof(unsigned char));
	receive(readSocket, receivedBuffer, receivedOffset);
	receivedOffset = 0;

	numInputs = inputCircuit -> numInputsExecutor;

	c_i_array = (struct u_v_Pair_ECC **) calloc(numInputs * 2, sizeof(struct u_v_Pair_ECC*));
	keyPairs = deserialise_PKs_otKeyPair_ECC_Array(receivedBuffer, numInputs);
	outputOffset = 0;

	inputGatesOffset = inputCircuit -> numInputsBuilder;

	#pragma omp parallel for private(tempWire, i, j, outputOffset) schedule(auto)
	for(i = inputGatesOffset; i < inputGatesOffset + numInputs; i ++)
	{
		j = i - inputGatesOffset;
		outputOffset = j * 2;
		tempWire = inputCircuit -> gates[i] -> outputWire;

		bulk_senderOT_UC_ECC(tempWire -> outputGarbleKeys -> key0, tempWire -> outputGarbleKeys -> key1, 16, params, state, keyPairs[j], c_i_array, outputOffset);
	}

	outputBuffer = serialise_U_V_Pair_ECC_Array(c_i_array, numInputs * 2, &outputOffset);

	sendBoth(writeSocket, outputBuffer, outputOffset);
}



void builder_side_OT(int writeSocket, int readSocket, struct decParams *params, struct Circuit *inputCircuit, gmp_randstate_t *state)
{
	struct u_v_Pair **c_i_array;
	struct otKeyPair **keyPairs;
	struct wire *tempWire;

	unsigned char *receivedBuffer, *outputBuffer;
	int receivedOffset = 0, outputOffset = 0, tempSize, numInputs = 0;
	int i, j, inputGatesOffset = 0;


	receivedOffset = receiveInt(readSocket);
	receivedBuffer = (unsigned char*) calloc(receivedOffset, sizeof(unsigned char));
	receive(readSocket, receivedBuffer, receivedOffset);
	receivedOffset = 0;

	numInputs = inputCircuit -> numInputsExecutor;

	c_i_array = (struct u_v_Pair **) calloc(numInputs * 2, sizeof(struct u_v_Pair*));
	keyPairs = deserialise_PKs_otKeyPair_Array(receivedBuffer, numInputs);
	outputOffset = 0;

	inputGatesOffset = inputCircuit -> numInputsBuilder;

	#pragma omp parallel for private(tempWire, i, j, outputOffset) schedule(auto)
	for(i = inputGatesOffset; i < inputGatesOffset + inputCircuit -> numInputsExecutor; i ++)
	{
		if( 0x00 == inputCircuit -> gates[i] -> outputWire -> wireOwner &&
			0x01 == (0x0F & inputCircuit -> gates[i] -> outputWire -> wireMask) )
		{
			j = i - inputGatesOffset;
			outputOffset = j * 2;
			tempWire = inputCircuit -> gates[i] -> outputWire;

			bulk_senderOT_UC(tempWire -> outputGarbleKeys -> key0, tempWire -> outputGarbleKeys -> key1, 16, params, state, keyPairs[j], c_i_array, outputOffset);
		}
	}

	outputBuffer = serialise_U_V_Pair_Array(c_i_array, numInputs * 2, &outputOffset);

	sendInt(writeSocket, outputOffset);
	send(writeSocket, outputBuffer, outputOffset);
}



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


	struct decParams_ECC *params = senderCRS_ECC_Syn_Dec(writeSocket, readSocket, 1024, *state);


	builder_side_OT_ECC(writeSocket, readSocket, params, inputCircuit, state);

	c_1 = clock();
	timestamp_1 = timestamp();

	printTiming(&timestamp_0, &timestamp_1, c_0, c_1, "OT - Sender");
}



// Function send a single gate. Not used but included for completeness.
void sendGate(struct gateOrWire *inputGW, int writeSocket, int readSocket)
{
	unsigned char *buffer, *lengthBuffer = (unsigned char*) calloc(4, sizeof(unsigned char));
	int bufferLength, j;

	sendInt(writeSocket, bufferLength);
	send(writeSocket, buffer, bufferLength);
	free(buffer);

}


// Send the full Circuit (gates, numGates, numOutputs etc.)
void sendCircuit(int writeSocket, int readSocket, struct Circuit *inputCircuit)
{
	unsigned char *bufferToSend, *circuitBuffer;
	int i, bufferLength, bufferOffset;
	int circuitLength = 0; 

	struct timespec timestamp_0 = timestamp(), timestamp_1;
	clock_t c_0, c_1;
	c_0 = clock();


	// Serialise the circuit.
	circuitBuffer = serialiseCircuit(inputCircuit, &circuitLength);

	// How big is buffer we're going to be sending, then calloc space.
	bufferLength = circuitLength + (6 * sizeof(int)) + (inputCircuit -> numGates * sizeof(int));
	bufferToSend = (unsigned char*) calloc(bufferLength, sizeof(unsigned char));


	// Copy Circuit parameters into the buffer we're going to send.
	memcpy(bufferToSend, &(inputCircuit -> numGates), sizeof(int));
	memcpy(bufferToSend + 1 * sizeof(int), &(inputCircuit -> numInputs), sizeof(int));
	memcpy(bufferToSend + 2 * sizeof(int), &(inputCircuit -> numOutputs), sizeof(int));
	memcpy(bufferToSend + 3 * sizeof(int), &(inputCircuit -> numInputsBuilder), sizeof(int));
	memcpy(bufferToSend + 4 * sizeof(int), &(inputCircuit -> numInputsExecutor), sizeof(int));
	memcpy(bufferToSend + 5 * sizeof(int), &(inputCircuit -> securityParam), sizeof(int));
	bufferOffset = 6 * sizeof(int);

	// Copy across the ExecOrder into the bufferToSend.
	memcpy(bufferToSend + bufferOffset, inputCircuit -> execOrder, inputCircuit -> numGates * sizeof(int));
	bufferOffset += (inputCircuit -> numGates * sizeof(int));

	// Copy across the Circuit into the bufferToSend.
	memcpy(bufferToSend + bufferOffset, circuitBuffer, circuitLength);

	// Send the buffer we've compiled.
	sendInt(writeSocket, bufferLength);
	send(writeSocket, bufferToSend, bufferLength);

	c_1 = clock();
	timestamp_1 = timestamp();

	printTiming(&timestamp_0, &timestamp_1, c_0, c_1, "Circuit Sending");

	free(bufferToSend);
	free(circuitBuffer);
}