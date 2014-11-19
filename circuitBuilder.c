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


void runCircuitBuilder( struct gateOrWire **inputCircuit, int numGates, int sockfd, mpz_t N )
{
	int i;

	for(i = 0; i < numGates; i ++)
	{
		if( 0x00 == inputCircuit[i] -> outputWire -> wireOwner &&
			0xF0 == inputCircuit[i] -> outputWire -> wireMask )
		{
			provideKeyForGate(inputCircuit[i], sockfd, N);
		}
	}
}


void sendGate(struct gateOrWire *inputGW, int writeSocket)
{
	unsigned char *buffer, *lengthBuffer = (unsigned char*) calloc(4, sizeof(unsigned char));
	int bufferLength, j;

	buffer = serialiseGateOrWire(inputGW, &bufferLength);
	sendInt(writeSocket, bufferLength);
	send(writeSocket, buffer, bufferLength);
	free(buffer);
	
	if(NULL != inputGW -> gatePayload)
	{
		for(j = 0; j < inputGW -> gatePayload -> outputTableSize; j ++)
		{
			send(writeSocket, (octet*)inputGW -> gatePayload -> encOutputTable[j], 32);
		}
	}
}


void sendCircuit(int writeSocket, struct gateOrWire **inputCircuit, int numGates, int *execOrder)
{
	unsigned char *execOrderBuffer;
	int i, bufferLength = numGates * sizeof(int);
	
	execOrderBuffer = (unsigned char*) calloc(bufferLength, sizeof(unsigned char));
	memcpy(execOrderBuffer, execOrder, numGates * sizeof(int));

	sendInt(writeSocket, numGates);

	sendInt(writeSocket, bufferLength);
	send(writeSocket, execOrderBuffer, bufferLength);

	for(i = 0; i < numGates; i ++)
	{
		sendGate(inputCircuit[i], writeSocket);
	}

	printf("Circuit sent.\n");
}
