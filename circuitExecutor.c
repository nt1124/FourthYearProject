void readInputLinesExec(int writeSocket, int readSocket,
						char *line, struct Circuit *inputCircuit,
						gmp_randstate_t *state, struct decParams *params)
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

	// tempBuffer = receiverOT_Toy(writeSocket, readSocket, value, &outputLength);
	// tempBuffer = receiverOT_SH_RSA(SKi, state, writeSocket, readSocket, value, &outputLength);

	tempBuffer = receiverOT_UC(writeSocket, readSocket, value, params, &outputLength, state);
	memcpy(inputCircuit -> gates[gateID] -> outputWire -> wireOutputKey, tempBuffer, 16);

	free(tempBuffer);
}


// Read the file containing our input data.
void readInputDetailsFileExec(int writeSocket, int readSocket, char *filepath, struct Circuit *inputCircuit)
{
	FILE *file = fopen ( filepath, "r" );
	gmp_randstate_t *state = seedRandGen();
	int i, outputLength = 0;
	unsigned char value, *tempBuffer;
	// struct rsaPrivKey *SKi = generatePrivRSAKey(*state);

	time_t t_0, t_1;
	clock_t c_0, c_1;
	t_0 = time(NULL);
	c_0 = clock();

	struct decParams *params = receiverCRS_Syn(writeSocket, readSocket, 1024, *state);


	if ( file != NULL )
	{
		char line [ 512 ];
		while ( fgets ( line, sizeof line, file ) != NULL )
		{
			readInputLinesExec(writeSocket, readSocket, line, inputCircuit, state, params);
		}
		fclose ( file );

		printf("++  %d\n", inputCircuit -> numInputs);
		fflush(stdout);
		for(i = 0; i < inputCircuit -> numInputs; i ++)
		{
			/*
			printf("+   %d\n", i);
			fflush(stdout);
			if(0x00 == inputCircuit -> gates[i] -> outputWire -> wireOwner)
			{
				printf(">>>   %d\n", i);
				fflush(stdout);
				value = inputCircuit -> gates[i] -> outputWire -> wirePermedValue;
				value = value ^ (inputCircuit -> gates[i] -> outputWire -> wirePerm & 0x01);
				tempBuffer = receiverOT_UC(writeSocket, readSocket, value, params, &outputLength, state);
				memcpy(inputCircuit -> gates[i] -> outputWire -> wireOutputKey, tempBuffer, 16);

				// free(tempBuffer);
			}
			*/
		}
	}

	t_1 = time(NULL);
	c_1 = clock();

	printf("\tOT wall clock time: %ld\n", (long) (t_1 - t_0));
	printf("\tOT CPU time:        %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
}


// Run the circuit, executor style.
void runCircuitExec( struct Circuit *inputCircuit, int writeSocket, int readSocket, char *filepath )
{
	unsigned char *tempBuffer;
	int i, gateID, outputLength = 0, j, nLength;

	printf("--  %d\n", inputCircuit -> numGates);
	printf("--  %d\n", inputCircuit -> numInputs);
	readInputDetailsFileExec(writeSocket, readSocket, filepath, inputCircuit);


	time_t t_0, t_1;
	t_0 = time(NULL);

	for(i = 0; i < inputCircuit -> numGates; i ++)
	{
		gateID = inputCircuit -> execOrder[i];
		if( NULL != inputCircuit -> gates[gateID] -> gatePayload )
		{
			// decryptGate(inputCircuit -> gates[gateID], inputCircuit -> gates);
			evaulateGate(inputCircuit -> gates[gateID], inputCircuit -> gates);
		}
	}
	t_1 = time(NULL);

	printf ("\tCircuit Execution wall clock time: %ld\n", (long) (t_1 - t_0));
}


// Receive the number of gates we can expect in this circuit.
int receiveNumGates(int writeSocket, int readSocket)
{
	int numGates;

	numGates = receiveInt(readSocket);

	return numGates;
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
struct gateOrWire **receiveCircuit(int numGates, int writeSocket, int readSocket)
{
	int i, j, bufferLength = 0;
	unsigned char *buffer = NULL;
	// struct gateOrWire **inputCircuit = (struct gateOrWire **) calloc(numGates, sizeof(struct gateOrWire*));

	printf("Circuit has %d gates!\n", numGates);

	bufferLength = receiveInt(readSocket);
	buffer = (unsigned char*) calloc(bufferLength, sizeof(unsigned char));
	receive(readSocket, buffer, bufferLength);

	return deserialiseCircuit(buffer, numGates);
	/*
	for(i = 0; i < numGates; i ++)
	{
		bufferLength = receiveInt(readSocket);
		buffer = (unsigned char*) calloc(bufferLength, sizeof(unsigned char));
		receive(readSocket, buffer, bufferLength);
		inputCircuit[i] = deserialiseGateOrWire(buffer);
		free(buffer);

		if(NULL != inputCircuit[i] -> gatePayload)
		{
			buffer = (unsigned char*) calloc(32, sizeof(unsigned char));
			for(j = 0; j < inputCircuit[i] -> gatePayload -> outputTableSize; j ++)
			{
				receive(readSocket, (octet*) buffer, 32);
				memcpy(inputCircuit[i] -> gatePayload -> encOutputTable[j], buffer, 32);
			}
			free(buffer);
		}
	}

	return inputCircuit;
	*/
}

