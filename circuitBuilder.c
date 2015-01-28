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
	int i;
	
	gmp_randstate_t *state = seedRandGen();
	struct decParams *params = senderCRS_Syn(writeSocket, readSocket);

	for(i = 0; i < numGates; i ++)
	{
		if( 0x00 == inputCircuit[i] -> outputWire -> wireOwner &&
			0x01 == (0x0F & inputCircuit[i] -> outputWire -> wireMask) )
		{
			tempWire = inputCircuit[i] -> outputWire;
			// senderOT_Toy(writeSocket, readSocket, tempWire -> outputGarbleKeys -> key0, tempWire -> outputGarbleKeys -> key1, 16);
			// senderOT_SH_RSA(writeSocket, readSocket, tempWire -> outputGarbleKeys -> key0, tempWire -> outputGarbleKeys -> key1, 16);
			senderOT_UC(writeSocket, readSocket, tempWire -> outputGarbleKeys -> key0, tempWire -> outputGarbleKeys -> key1, 16, params, state);
		}
	}
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


void sendCircuit(int writeSocket, int readSocket, struct gateOrWire **inputCircuit, int numGates, int *execOrder)
{
	unsigned char *bufferToSend;
	int i, bufferLength = numGates * sizeof(int);
	
	bufferToSend = (unsigned char*) calloc(bufferLength, sizeof(unsigned char));
	memcpy(bufferToSend, execOrder, numGates * sizeof(int));

	sendInt(writeSocket, numGates);

	sendInt(writeSocket, bufferLength);
	send(writeSocket, bufferToSend, bufferLength);

	free(bufferToSend);

	bufferToSend = 0;
	bufferToSend = serialiseCircuit(inputCircuit, numGates, &bufferLength);
	/*
	for(i = 0; i < numGates; i ++)
	{
		sendGate(inputCircuit[i], writeSocket, readSocket);
	}
	*/
	sendInt(writeSocket, bufferLength);
	send(writeSocket, bufferToSend, bufferLength);

	printf("Circuit sent.\n");
}