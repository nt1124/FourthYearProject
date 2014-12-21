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



void readInputDetailsFileExec(int writeSocket, int readSocket, char *filepath, struct Circuit *inputCircuit)
{
	FILE *file = fopen ( filepath, "r" );
	gmp_randstate_t *state = seedRandGen();
	// struct rsaPrivKey *SKi = generatePrivRSAKey(*state);
	struct decParams *params = receiverCRS_Syn(writeSocket, readSocket, 1024, *state);

	if ( file != NULL )
	{
		char line [ 512 ];
		while ( fgets ( line, sizeof line, file ) != NULL )
		{
			readInputLinesExec(writeSocket, readSocket, line, inputCircuit, state, params);
		}
		fclose ( file );
	}
}



void runCircuitExec( struct Circuit *inputCircuit, int writeSocket, int readSocket, char *filepath )
{
	unsigned char *tempBuffer;
	int i, gateID, outputLength = 0, j, nLength;

	readInputDetailsFileExec(writeSocket, readSocket, filepath, inputCircuit);

	for(i = 0; i < inputCircuit -> numGates; i ++)
	{
		gateID = inputCircuit -> execOrder[i];
		if( NULL != inputCircuit -> gates[gateID] -> gatePayload )
		{
			decryptGate(inputCircuit -> gates[gateID], inputCircuit -> gates);
		}
	}
}



int receiveNumGates(int writeSocket, int readSocket)
{
	int numGates;

	numGates = receiveInt(readSocket);

	return numGates;
}



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



struct gateOrWire **receiveCircuit(int numGates, int writeSocket, int readSocket)
{
	int i, j, bufferLength = 0;
	unsigned char *buffer = NULL;
	struct gateOrWire **inputCircuit;

	inputCircuit = (struct gateOrWire **) calloc(numGates, sizeof(struct gateOrWire*));

	printf("Circuit has %d gates!\n", numGates);

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
}

