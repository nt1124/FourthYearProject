// Function for reading in an input file when circuit has been altered to encode the Executors
// input in the fashion suggested in Lindell/Pinkas 2007
void readInputLinesExec_CnC(struct Circuit *inputCircuit, int inputLineIndex, unsigned char value)
{
	int gateID = 0, i, j;
	int startIndex = inputCircuit -> numInputsBuilder + inputLineIndex * inputCircuit -> securityParam;
	int numBytesToGenerate;
	unsigned char *tempBuffer;
	unsigned char xorSum = 0x00, tempValue;
	struct wire *outputWire;


	numBytesToGenerate = (inputCircuit -> securityParam / 8) + 1;
	tempBuffer = generateRandBytes(numBytesToGenerate, numBytesToGenerate);

	for(i = 0; i < inputCircuit -> securityParam - 1; i ++)
	{
		tempValue = 0x01 & (tempBuffer[j] << (i % 8));
		xorSum ^= tempValue;

		tempValue ^= (inputCircuit -> gates[startIndex] -> outputWire -> wirePerm & 0x01);
		inputCircuit -> gates[startIndex] -> outputWire -> wirePermedValue = tempValue;

		startIndex ++;
		if(7 == i % 7)
		{
			j ++;
		}
	}

	tempValue = value ^ xorSum;
	inputCircuit -> gates[startIndex] -> outputWire -> wirePermedValue = tempValue ^ (inputCircuit -> gates[startIndex] -> outputWire -> wirePerm & 0x01);
}


void readInputLinesExec(char *line, struct Circuit *inputCircuit, int inputLineIndex)
{
	struct wire *outputWire;
	int strIndex = 0, gateID = 0, outputLength = 0, i, j;
	unsigned char value;

	while( ' ' != line[strIndex++] ){}
	while( ' ' != line[strIndex] )
	{
		gateID *= 10;
		gateID += line[strIndex] - 48;
		strIndex ++;
	}
	strIndex ++;

	if( '1' == line[strIndex] )
		value = 0x01;
	else if( '0' == line[strIndex] )
		value = 0x00;

	outputWire = inputCircuit -> gates[gateID] -> outputWire;

	if(1 == inputCircuit -> securityParam)
	{
		outputWire -> wirePermedValue = value ^ (outputWire -> wirePerm & 0x01);
	}
	else
	{
		readInputLinesExec_CnC(inputCircuit, inputLineIndex, value);
	}
}


void readLocalExec(char *filepath, struct Circuit *inputCircuit)
{
	FILE *file = fopen( filepath, "r" );
	int i = 0;

	if ( file != NULL )
	{
		char line [ 512 ];
		while ( fgets ( line, sizeof line, file ) != NULL )
		{
			readInputLinesExec(line, inputCircuit, i);
			i ++;
		}
		fclose ( file );
	}
}


void executor_side_OT(int writeSocket, int readSocket,
					struct decParams *params, struct Circuit *inputCircuit,
					gmp_randstate_t *state)
{
	struct otKeyPair **otKeyPairs;
	int tempSize = 0, i, j = 0, bufferOffset = 0, outputLength, idOffset;
	unsigned char *tempBuffer, *commBuffer;
	unsigned char value;



	tempSize = inputCircuit -> numInputsExecutor;

	otKeyPairs = (struct otKeyPair **) calloc(tempSize, sizeof(struct otKeyPair *));


	for(i = 0; i < inputCircuit -> numGates; i ++)
	{
		if(0x00 == inputCircuit -> gates[i] -> outputWire -> wireOwner &&
			0x01 == (0x0F & inputCircuit -> gates[i] -> outputWire -> wireMask))
		{
			value = inputCircuit -> gates[i] -> outputWire -> wirePermedValue;
			value = value ^ (inputCircuit -> gates[i] -> outputWire -> wirePerm & 0x01);

			otKeyPairs[j++] = bulk_one_receiverOT_UC(value, params, state);
		}
	}

	commBuffer = serialise_PKs_otKeyPair_Array(otKeyPairs, tempSize, &bufferOffset);
	sendInt(writeSocket, bufferOffset);
	send(writeSocket, commBuffer, bufferOffset);
	free(commBuffer);

	bufferOffset = receiveInt(readSocket);
	commBuffer = (unsigned char*) calloc(bufferOffset, sizeof(unsigned char));
	receive(readSocket, commBuffer, bufferOffset);


	bufferOffset = 0;
	j = 0;
	idOffset = inputCircuit -> numInputsBuilder;

	// #pragma omp parallel for private(tempWire, i, j, outputOffset) schedule(auto)
	for(i = idOffset; i < idOffset + inputCircuit -> numInputsBuilder; i ++)
	{
		if(0x00 == inputCircuit -> gates[i] -> outputWire -> wireOwner &&
			0x01 == (0x0F & inputCircuit -> gates[i] -> outputWire -> wireMask))
		{
			value = inputCircuit -> gates[i] -> outputWire -> wirePermedValue;
			value = value ^ (inputCircuit -> gates[i] -> outputWire -> wirePerm & 0x01);

			tempBuffer = bulk_two_receiverOT_UC(commBuffer, &bufferOffset, otKeyPairs[j++], params, value, &outputLength);
			memcpy(inputCircuit -> gates[i] -> outputWire -> wireOutputKey, tempBuffer, 16);
			free(tempBuffer);
		}
	}
}


// Read the file containing our input data.
void readInputDetailsFileExec(int writeSocket, int readSocket, char *filepath, struct Circuit *inputCircuit)
{
	FILE *file = fopen( filepath, "r" );
	gmp_randstate_t *state = seedRandGen();
	int i = 0, outputLength = 0, tempSize = 0, j = 0;
	unsigned char value, *tempBuffer;

	unsigned char *receivedBuffer, *toSendBuffer;
	int bufferOffset = 0;

	struct timespec timestamp_0 = timestamp(), timestamp_1;
	clock_t c_0, c_1;
	c_0 = clock();

	struct otKeyPair **otKeyPairs;
	struct decParams *params = receiverCRS_Syn_Dec(writeSocket, readSocket);//, 1024, *state);


	if ( file != NULL )
	{
		char line [ 512 ];
		while ( fgets ( line, sizeof line, file ) != NULL )
		{
			readInputLinesExec(line, inputCircuit, i);
			i ++;
		}
		fclose ( file );

		executor_side_OT(writeSocket, readSocket, params, inputCircuit, state);
	}


	c_1 = clock();
	timestamp_1 = timestamp();

    printTiming(&timestamp_0, &timestamp_1, c_0, c_1, "OT - Receiver");
}


// Run the circuit, executor style.
void runCircuitExec( struct Circuit *inputCircuit, int writeSocket, int readSocket, char *filepath )
{
	int i, gateID;
	struct timespec timestamp_0, timestamp_1;
	clock_t c_0, c_1;

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


// Receive the actual circuit. Stand alone, purely deals with receiving the gates. Not used.
struct gateOrWire **receiveGatesOfCircuitAlt(int numGates, int writeSocket, int readSocket)
{
	struct timespec timestamp_0 = timestamp(), timestamp_1;
	clock_t c_0, c_1;
	c_0 = clock();


	int i, j, bufferLength = 0;
	unsigned char *buffer = NULL;
	struct gateOrWire **inputCircuit;

	// printf("Circuit has %d gates!\n", numGates);

	bufferLength = receiveInt(readSocket);
	buffer = (unsigned char*) calloc(bufferLength, sizeof(unsigned char));
	receive(readSocket, buffer, bufferLength);


	inputCircuit = deserialiseCircuit(buffer, numGates);


	c_1 = clock();
	timestamp_1 = timestamp();

    printTiming(&timestamp_0, &timestamp_1, c_0, c_1, "Receiving Circuit");

	return inputCircuit;
}


// Receive the actual circuit.
struct gateOrWire **receiveGatesOfCircuit(unsigned char *inputBuffer, int numGates)
{
	int i, j;
	struct gateOrWire **inputCircuit;
	struct timespec timestamp_0 = timestamp(), timestamp_1;
	clock_t c_0, c_1;
	c_0 = clock();


	printf("Circuit has %d gates!\n\n", numGates);


	inputCircuit = deserialiseCircuit(inputBuffer, numGates);


	c_1 = clock();
	timestamp_1 = timestamp();
	printTiming(&timestamp_0, &timestamp_1, c_0, c_1, "Receiving Gates of Circuit");

	return inputCircuit;
}


struct Circuit *receiveFullCircuit(int writeSocket, int readSocket)
{
	struct Circuit *inputCircuit = (struct Circuit*) calloc(1, sizeof(struct Circuit));
	unsigned char *receivedBuffer;
	int bufferLength, bufferOffset = 6 * sizeof(int);

	bufferLength = receiveInt(readSocket);
	receivedBuffer = (unsigned char*) calloc(bufferLength, sizeof(unsigned char));
	receive(readSocket, receivedBuffer, bufferLength);


	memcpy(&(inputCircuit -> numGates), receivedBuffer, sizeof(int));
	memcpy(&(inputCircuit -> numInputs), receivedBuffer + sizeof(int), sizeof(int));
	memcpy(&(inputCircuit -> numOutputs), receivedBuffer + 2 * sizeof(int), sizeof(int));
	memcpy(&(inputCircuit -> numInputsBuilder), receivedBuffer + 3 * sizeof(int), sizeof(int));
	memcpy(&(inputCircuit -> numInputsExecutor), receivedBuffer + 4 * sizeof(int), sizeof(int));
	memcpy(&(inputCircuit -> securityParam), receivedBuffer + 5 * sizeof(int), sizeof(int));


	// Get the ExecOrder.
	inputCircuit -> execOrder = (int*) calloc(inputCircuit -> numGates, sizeof(int));
	memcpy(inputCircuit -> execOrder, receivedBuffer + bufferOffset, inputCircuit -> numGates * sizeof(int));
	bufferOffset += (inputCircuit -> numGates * sizeof(int));

	inputCircuit -> gates = receiveGatesOfCircuit(receivedBuffer + bufferOffset, inputCircuit -> numGates);

	return inputCircuit;
}