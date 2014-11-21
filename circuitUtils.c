void printAllOutput(struct gateOrWire **inputCircuit, int numGates)
{
	int i;
	unsigned char tempBit;

	for(i = 0; i < numGates; i ++)
	{
		if( 0x0F == inputCircuit[i] -> outputWire -> wireMask )
		{
			tempBit = inputCircuit[i] -> outputWire -> wirePermedValue;
			tempBit = tempBit ^ (0x01 & inputCircuit[i] -> outputWire -> wirePerm);
			printf("Gate %d = %d\n", inputCircuit[i] -> G_ID, tempBit);
		}
	}
}


<<<<<<< HEAD
=======

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


void readInputLinesExec(int sockfd, char *line, struct gateOrWire **inputCircuit,
									gmp_randstate_t *state, struct rsaPrivKey *SKi)
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

	inputCircuit[gateID] -> outputWire -> wirePermedValue = value ^ (inputCircuit[gateID] -> outputWire -> wirePerm & 0x01);

	tempBuffer = receiverOT_Toy(sockfd, value, &outputLength);
	// tempBuffer = receiverOT_SH_RSA(SKi, state, sockfd, value, &outputLength);
	memcpy(inputCircuit[gateID] -> outputWire -> wireOutputKey, tempBuffer, 16);

	free(tempBuffer);
}


void readInputDetailsFileExec(int sockfd, char *filepath, struct gateOrWire **inputCircuit,
											gmp_randstate_t *state, struct rsaPrivKey *SKi)
{
	FILE *file = fopen ( filepath, "r" );

	if ( file != NULL )
	{
		char line [ 512 ];
		while ( fgets ( line, sizeof line, file ) != NULL )
		{
			readInputLinesExec(sockfd, line, inputCircuit, state, SKi);
		}
		fclose ( file );
	}
}


void runCircuitExec( struct gateOrWire **inputCircuit, int numGates, int sockfd, char *filepath, int *execOrder )
{
	unsigned char *tempBuffer;
	int i, gateID, outputLength = 0, j;
	gmp_randstate_t *state = seedRandGen();
	struct rsaPrivKey *SKi = generatePrivRSAKey(*state);

	readInputDetailsFileExec(sockfd, filepath, inputCircuit, state, SKi);

	for(i = 0; i < numGates; i ++)
	{
		gateID = execOrder[i];
		if( NULL != inputCircuit[gateID] -> gatePayload )
		{
			decryptGate(inputCircuit[gateID], inputCircuit);
		}
	}
}


void runCircuitBuilder( struct gateOrWire **inputCircuit, int numGates, int sockfd )
{
	int i;

	for(i = 0; i < numGates; i ++)
	{
		if( 0x00 == inputCircuit[i] -> outputWire -> wireOwner &&
			0xF0 == inputCircuit[i] -> outputWire -> wireMask )
		{
			provideKeyForGate(inputCircuit[i], sockfd);
		}
	}
}


>>>>>>> parent of 402c8f5... Ready for multi machien test
void runCircuitLocal( struct gateOrWire **inputCircuit, int numGates, int *execOrder )
{
	int i, j, gateID;

	for(i = 0; i < numGates; i ++)
	{
		gateID = execOrder[i];
		if( NULL != inputCircuit[gateID] -> gatePayload )
		{
			decryptGate(inputCircuit[gateID], inputCircuit);
		}
	}
}
