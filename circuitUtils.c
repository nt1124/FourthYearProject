void outputAsBinaryString(struct gateOrWire **inputCircuit, int numGates, int *numOutputs)
{
	int i, j = 0, k;
	unsigned char *binaryOutput, *hexOutput;
	unsigned char tempBit, tempHex;
	*numOutputs = 0;

	for(i = 0; i < numGates; i ++)
	{
		if( 0x0F == inputCircuit[i] -> outputWire -> wireMask )
			(*numOutputs) ++;
	}

	binaryOutput = (unsigned char*) calloc(*numOutputs, sizeof(unsigned char));

	for(i = 0; i < numGates; i ++)
	{
		if( 0x0F == inputCircuit[i] -> outputWire -> wireMask )
		{
			tempBit = inputCircuit[i] -> outputWire -> wirePermedValue;
			binaryOutput[j++] = tempBit ^ (0x01 & inputCircuit[i] -> outputWire -> wirePerm);
		}
	}

	hexOutput = (unsigned char*) calloc( (*numOutputs / 8) + 2, sizeof(unsigned char) );

	j = 0;
	k = 7;
	tempHex = 0;
	for(i = 0; i < *numOutputs; i ++)
	{
		hexOutput[j] += (binaryOutput[i] << k);
		k --;

		if(7 == i % 8)
		{
			j++;
			k = 7;
		}
	}

	printf("Candidate output as Hex: ");
	for(i = 0; i < j; i ++)
	{
		printf("%02X", hexOutput[i]);
	}
	printf("\n");
}


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


void readInputLinesExec(int writeSocket, int readSocket,
						char *line, struct gateOrWire **inputCircuit,
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

	inputCircuit[gateID] -> outputWire -> wirePermedValue = value ^ (inputCircuit[gateID] -> outputWire -> wirePerm & 0x01);

	// tempBuffer = receiverOT_Toy(sockfd, value, &outputLength);
	// tempBuffer = receiverOT_SH_RSA(SKi, state, writeSocket, readSocket, value, &outputLength);
	tempBuffer = receiverOT_UC(writeSocket, readSocket, value, params, &outputLength, state);
	memcpy(inputCircuit[gateID] -> outputWire -> wireOutputKey, tempBuffer, 16);

	free(tempBuffer);
}


void readInputDetailsFileExec(int writeSocket, int readSocket, char *filepath, struct gateOrWire **inputCircuit)
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


void runCircuitExec( struct gateOrWire **inputCircuit, int numGates, int writeSocket, int readSocket, char *filepath, int *execOrder )
{
	unsigned char *tempBuffer;
	int i, gateID, outputLength = 0, j, nLength;

	readInputDetailsFileExec(writeSocket, readSocket, filepath, inputCircuit);

	for(i = 0; i < numGates; i ++)
	{
		gateID = execOrder[i];
		if( NULL != inputCircuit[gateID] -> gatePayload )
		{
			decryptGate(inputCircuit[gateID], inputCircuit);
		}
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
			0xF0 == inputCircuit[i] -> outputWire -> wireMask )
		{
			tempWire = inputCircuit[i] -> outputWire;
			// senderOT_Toy(writeSocket, tempWire -> outputGarbleKeys -> key0, tempWire -> outputGarbleKeys -> key1, 16);
			// senderOT_SH_RSA(writeSocket, readSocket, tempWire -> outputGarbleKeys -> key0, tempWire -> outputGarbleKeys -> key1, 16);
			senderOT_UC(writeSocket, readSocket, tempWire -> outputGarbleKeys -> key0, tempWire -> outputGarbleKeys -> key1, 16, params, state);
		}
	}
}


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


void sendGate(struct gateOrWire *inputGW, int writeSocket, int readSocket)
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


void sendCircuit(int writeSocket, int readSocket, struct gateOrWire **inputCircuit, int numGates, int *execOrder)
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
		sendGate(inputCircuit[i], writeSocket, readSocket);
	}

	printf("Circuit sent.\n");
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