void readInputLinesExec(int writeSocket, int readSocket,
						char *line, struct Circuit *inputCircuit)
{
	int strIndex = 0, gateID = 0, outputLength = 0, i;
	unsigned char *tempBuffer;
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

	inputCircuit -> gates[gateID] -> outputWire -> wirePermedValue = value ^ (inputCircuit -> gates[gateID] -> outputWire -> wirePerm & 0x01);
}


// Read the file containing our input data.
void readInputDetailsFileExec(int writeSocket, int readSocket, char *filepath, struct Circuit *inputCircuit)
{
	FILE *file = fopen( filepath, "r" );
	gmp_randstate_t *state = seedRandGen();
	int i, outputLength = 0, tempSize = 0, j = 0;
	unsigned char value, *tempBuffer;
	// struct rsaPrivKey *SKi = generatePrivRSAKey(*state);

	unsigned char *receivedBuffer, *toSendBuffer;
	int bufferOffset = 0;

	struct timespec timestamp_0 = timestamp(), timestamp_1;
	clock_t c_0, c_1;
	c_0 = clock();

	struct otKeyPair **otKeyPairs;
	struct decParams *params = receiverCRS_Syn(writeSocket, readSocket, 1024, *state);


	if ( file != NULL )
	{
		char line [ 512 ];
		while ( fgets ( line, sizeof line, file ) != NULL )
		{
			readInputLinesExec(writeSocket, readSocket, line, inputCircuit);
		}
		fclose ( file );

		for(i = 0; i < inputCircuit -> numGates; i ++)
		{
			if(0x00 == inputCircuit -> gates[i] -> outputWire -> wireOwner &&
				0x01 == (0x0F & inputCircuit -> gates[i] -> outputWire -> wireMask))
			{
				tempSize ++;
			}
		}

		otKeyPairs = (struct otKeyPair **) calloc(tempSize, sizeof(struct otKeyPair *));
		tempSize *= 2 * (136 + 4);
		toSendBuffer = (unsigned char *) calloc(tempSize, sizeof(unsigned char));

		for(i = 0; i < inputCircuit -> numGates; i ++)
		{
			if(0x00 == inputCircuit -> gates[i] -> outputWire -> wireOwner &&
				0x01 == (0x0F & inputCircuit -> gates[i] -> outputWire -> wireMask))
			{
				otKeyPairs[j++] = bulk_one_receiverOT_UC(value, params, state, toSendBuffer, &bufferOffset);
			}
		}

		sendInt(writeSocket, bufferOffset);
		send(writeSocket, toSendBuffer, bufferOffset);

		bufferOffset = receiveInt(readSocket);
		receivedBuffer = (unsigned char*) calloc(bufferOffset, sizeof(unsigned char));
		receive(readSocket, receivedBuffer, bufferOffset);


		bufferOffset = 0;
		j = 0;
		for(i = 0; i < inputCircuit -> numGates; i ++)
		{
			if(0x00 == inputCircuit -> gates[i] -> outputWire -> wireOwner &&
				0x01 == (0x0F & inputCircuit -> gates[i] -> outputWire -> wireMask))
			{
				value = inputCircuit -> gates[i] -> outputWire -> wirePermedValue;
				value = value ^ (inputCircuit -> gates[i] -> outputWire -> wirePerm & 0x01);

				tempBuffer = bulk_two_receiverOT_UC(receivedBuffer, &bufferOffset, otKeyPairs[j++], params, value, &outputLength);
				memcpy(inputCircuit -> gates[i] -> outputWire -> wireOutputKey, tempBuffer, 16);
				free(tempBuffer);
			}
		}
	}


	c_1 = clock();
	timestamp_1 = timestamp();

	printf("\nOT CPU time    :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
	printf("OT Custom time :     %lf\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));
}


// Run the circuit, executor style.
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

	printf("\nCircuit Evalutation CPU time    :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
	printf("Circuit Evalutation Custom time :     %lf\n\n", seconds_timespecDiff(&timestamp_0, &timestamp_1));
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
//struct gateOrWire **
void receiveCircuit(struct Circuit *inputCircuit, int writeSocket, int readSocket)
{
	struct timespec timestamp_0 = timestamp(), timestamp_1;
	clock_t c_0, c_1;
	c_0 = clock();


	int i, j, bufferLength = 0;
	unsigned char *buffer = NULL;
	// struct gateOrWire **gates;

	/*
	bufferLength = receiveInt(readSocket);
	buffer = (unsigned char*) calloc(bufferLength, sizeof(unsigned char));
	receive(readSocket, buffer, bufferLength);

	memcpy(&(inputCircuit -> numGates), buffer + 0 * sizeof(int), sizeof(int));
	memcpy(&(inputCircuit -> numInputs), buffer + 1 * sizeof(int), sizeof(int));
	memcpy(&(inputCircuit -> numOutputs), buffer + 2 * sizeof(int), sizeof(int));
	memcpy(&bufferLength, buffer + 3 * sizeof(int), sizeof(int));

	memcpy(inputCircuit -> execOrder, buffer + 4 * sizeof(int), inputCircuit -> numGates * sizeof(int));

	free(buffer);
	*/

	printf("Circuit has %d gates!\n", inputCircuit -> numGates);

	bufferLength = receiveInt(readSocket);
	buffer = (unsigned char*) calloc(bufferLength, sizeof(unsigned char));
	receive(readSocket, buffer, bufferLength);

	inputCircuit -> gates = deserialiseCircuit(buffer + 4 * sizeof(int), inputCircuit -> numGates, inputCircuit -> numInputs);


	c_1 = clock();
	timestamp_1 = timestamp();
	double temp = seconds_timespecDiff(&timestamp_0, &timestamp_1);

	printf("\nReceived Circuit CPU time    :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
	printf("Received Circuit Custom time :     %lf\n", temp);


	free(buffer);
}

