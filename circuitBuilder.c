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


void runCircuitBuilder( struct gateOrWire **inputCircuit, int numGates, int writeSocket, int readSocket)
{
	struct wire *tempWire;
	int i, numInputs = 0, tempSize;

	unsigned char *receivedBuffer, *outputBuffer;
	int receivedOffset = 0, outputOffset = 0;


	struct timespec timestamp_0 = timestamp(), timestamp_1;
	clock_t c_0, c_1;
	c_0 = clock();

	gmp_randstate_t *state = seedRandGen();
	struct decParams *params = senderCRS_Syn(writeSocket, readSocket);


	receivedOffset = receiveInt(readSocket);
	receivedBuffer = (unsigned char*) calloc(receivedOffset, sizeof(unsigned char));
	receive(readSocket, receivedBuffer, receivedOffset);
	receivedOffset = 0;

	for(i = 0; i < numGates; i ++)
	{
		if( 0x00 == inputCircuit[i] -> outputWire -> wireOwner &&
			0x01 == (0x0F & inputCircuit[i] -> outputWire -> wireMask) )
		{
			numInputs ++;
		}
	}

	tempSize = 4 * numInputs * (136 + 4);
	outputBuffer = (unsigned char *) calloc(tempSize, sizeof(unsigned char));

	for(i = 0; i < numGates; i ++)
	{
		if( 0x00 == inputCircuit[i] -> outputWire -> wireOwner &&
			0x01 == (0x0F & inputCircuit[i] -> outputWire -> wireMask) )
		{
			tempWire = inputCircuit[i] -> outputWire;

			bulk_senderOT_UC(tempWire -> outputGarbleKeys -> key0, tempWire -> outputGarbleKeys -> key1, 16, params, state,
							receivedBuffer, &receivedOffset, outputBuffer, &outputOffset);
		}
	}

	sendInt(writeSocket, outputOffset);
	send(writeSocket, outputBuffer, outputOffset);


	c_1 = clock();
	timestamp_1 = timestamp();
	double temp = seconds_timespecDiff(&timestamp_0, &timestamp_1);

	printf("\nSender OT CPU time  : %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
	printf("Sender OT Wall time : %lf\n", temp);
}



/*
void sendGate(struct gateOrWire *inputGW, int writeSocket, int readSocket)
{
	unsigned char *buffer, *lengthBuffer = (unsigned char*) calloc(4, sizeof(unsigned char));
	int bufferLength, j;

	//buffer = serialiseGateOrWire(inputGW, &bufferLength);
	sendInt(writeSocket, bufferLength);
	send(writeSocket, buffer, bufferLength);
	free(buffer);

	// This bit of code was a hack fix. Will not be needed when bulk sending?
	if(NULL != inputGW -> gatePayload)
	{
		for(j = 0; j < inputGW -> gatePayload -> outputTableSize; j ++)
		{
			send(writeSocket, (octet*)inputGW -> gatePayload -> encOutputTable[j], 32);
		}
	}
}
*/


void sendCircuit(int writeSocket, int readSocket, struct Circuit *inputCircuit)
{
	unsigned char *bufferToSend;
	int i, bufferLength = inputCircuit -> numGates * sizeof(int);

	struct timespec timestamp_0 = timestamp(), timestamp_1;
	clock_t c_0, c_1;
	c_0 = clock();
	
	bufferToSend = (unsigned char*) calloc(bufferLength, sizeof(unsigned char));
	memcpy(bufferToSend, inputCircuit -> execOrder, inputCircuit -> numGates * sizeof(int));

	sendInt(writeSocket, inputCircuit -> numGates);

	sendInt(writeSocket, bufferLength);
	send(writeSocket, bufferToSend, bufferLength);

	free(bufferToSend);

	bufferToSend = 0;
	bufferToSend = serialiseCircuit(inputCircuit -> gates, inputCircuit -> numGates, &bufferLength);

	sendInt(writeSocket, bufferLength);
	send(writeSocket, bufferToSend, bufferLength);

	c_1 = clock();
	timestamp_1 = timestamp();
	double temp = seconds_timespecDiff(&timestamp_0, &timestamp_1);
	printf("Circuit sent.\n");

	printf("\nCircuit Sending CPU time  :     %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
	printf("Circuit Sending Wall time :     %lf\n", temp);
}