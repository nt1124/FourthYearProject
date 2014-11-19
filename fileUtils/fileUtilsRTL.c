// Function for use in testing, will set all inputs to zero and
// initialise accordingly.
void zeroAllInputs(struct gateOrWire **inputCircuit, int numGates)
{
	struct wire* outputWire;
	int i;

	for(i = 0; i < numGates; i ++)
	{
		outputWire = inputCircuit[i] -> outputWire;

		outputWire -> wireOwner = 0xFF;
		outputWire -> wirePermedValue = outputWire -> outputGarbleKeys -> key0[16];
		memcpy(outputWire -> wireOutputKey, outputWire -> outputGarbleKeys -> key0, 16);
		
		if(0xF0 != inputCircuit[i] -> outputWire -> wireMask)
			break;
	}
}


// Returns an integer value when given a string with index pointing to start
// of the integer to read. Assumes delimtation by spaces.
int getIntFromString(char *inputStr, int& strIndex)
{
	int toReturn = 0;

	while(' ' != inputStr[strIndex])
	{
		toReturn *= 10;
		toReturn += (inputStr[strIndex] - 48);
		strIndex ++;
	}

	while(' ' == inputStr[strIndex])
	{
		strIndex ++;
	}

	return toReturn;
}


/*
int *getRawOutputTable(char gateType, int *inputIDs)
{
	int *toReturn = (int*) calloc(4, sizeof(int));
	
	if('A' == gateType)
	{
		toReturn[3] = 1;

		if(0 > inputIDs[0])
		{
			toReturn[0] = 1;
			toReturn[1] = 1;
		}
		if(0 > inputIDs[1])
		{
			toReturn[2] = 1;
			toReturn[3] = 0;
		}
	}
	else if('X' == gateType)
	{
		toReturn[1] = 1;
		toReturn[2] = 1;

		if(0 > inputIDs[0])
		{
			toReturn[0] = 1;
			toReturn[1] = 0;
		}
		if(0 > inputIDs[1])
		{
			toReturn[2] = 0;
			toReturn[3] = 1;
		}
	}

	return toReturn;
}
*/


struct gate *processGateRTL(int numInputWires, int *inputIDs, char gateType)
{
	struct gate *toReturn = (struct gate*) calloc(1, sizeof(struct gate));
	int i, outputTableSize = 1;

	toReturn -> numInputs = numInputWires;
	toReturn -> inputIDs = inputIDs;
	for(i = 0; i < toReturn -> numInputs; i ++)
		outputTableSize *= 2;

	toReturn -> outputTableSize = outputTableSize;
	
	toReturn -> rawOutputTable = (int*) calloc(4, sizeof(int));
	if('I' == gateType)
	{
		toReturn -> rawOutputTable = (int*) calloc(2, sizeof(int));
		toReturn -> rawOutputTable[0] = 1;
	}
	else
	{	
		if('1' == gateType)
		{
			toReturn -> rawOutputTable[3] = 1;
		}
		else if('2' == gateType)
		{
			toReturn -> rawOutputTable[2] = 1;
		}
		else if('3' == gateType)
		{
			toReturn -> rawOutputTable[1] = 1;
		}
		else if('4' == gateType)
		{
			toReturn -> rawOutputTable[0] = 1;
		}
		else if('X' == gateType)
		{
			toReturn -> rawOutputTable[1] = 1;
			toReturn -> rawOutputTable[2] = 1;
		}
		else if('N' == gateType)
		{
			toReturn -> rawOutputTable[0] = 1;
			toReturn -> rawOutputTable[3] = 1;
		}
	}
	// toReturn -> rawOutputTable = getRawOutputTable(gateType, inputIDs, circuit)

	toReturn -> encOutputTable = recursiveOutputTableAlt(toReturn);

	return toReturn;
}


struct gateOrWire *processGateOrWireRTL(int idNum, int *inputIDs, int numInputWires,
										char gateType, struct gateOrWire **circuit)
{
	struct gateOrWire *toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));

	toReturn -> G_ID = idNum;
	toReturn -> outputWire = (struct wire *) calloc(1, sizeof(struct wire));
	toReturn -> outputWire -> wirePerm = getPermutation();
	toReturn -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));
	toReturn -> outputWire -> outputGarbleKeys = generateGarbleKeyPair(toReturn -> outputWire -> wirePerm);

	toReturn -> gatePayload = processGateRTL(numInputWires, inputIDs, gateType);

	encWholeOutTable(toReturn, circuit);

	return toReturn;
}


struct gateOrWire *processGateLineRTL(char *line, struct gateOrWire **circuit)
{
	int strIndex = 0, idNum, i;
	int numInputWires, purposelessNumber;
	int *inputIDs;
	unsigned char gateType;

	numInputWires = getIntFromString(line, strIndex);

	// As of yet unsure of the purpose of this number. Ask Nigel.
	purposelessNumber = getIntFromString(line, strIndex);

	inputIDs = (int*) calloc(numInputWires, sizeof(int));
	for(i = 0; i < numInputWires; i ++)
	{
		inputIDs[i] = getIntFromString(line, strIndex);
	}

	idNum = getIntFromString(line, strIndex);
	gateType = line[strIndex];
	fflush(stdout);

	if('A' == gateType)
	{
		gateType = line[strIndex + 3];
	}

	return processGateOrWireRTL(idNum, inputIDs, numInputWires, gateType, circuit);
}


struct gateOrWire *initialiseInputWire(int idNum, unsigned char owner)
{
	struct gateOrWire *toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));

	toReturn -> G_ID = idNum;
	toReturn -> outputWire = (struct wire *) calloc(1, sizeof(struct wire));
	toReturn -> outputWire -> wirePerm = getPermutation();
	toReturn -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));
	toReturn -> outputWire -> outputGarbleKeys = generateGarbleKeyPair(toReturn -> outputWire -> wirePerm);
	toReturn -> gatePayload = NULL;
	toReturn -> outputWire -> wireMask = 0xF0;
	toReturn -> outputWire -> wireOwner = owner;

	return toReturn;
}


// FOR NOW we assume party 1 is building the circuit.
struct gateOrWire **initialiseAllInputs(int numGates, int numInputs1, int numInputs2, int **execOrder)
{
	struct gateOrWire **circuit = (struct gateOrWire**) calloc(numGates, sizeof(struct gateOrWire*));
	int i;

	for(i = 0; i < numInputs1; i ++)
	{
		circuit[i] = initialiseInputWire(i, 0xFF);
		(*execOrder)[i] = i;
	}

	for(i = numInputs2; i < numInputs1 + numInputs2; i ++)
	{
		circuit[i] = initialiseInputWire(i, 0x00);
		(*execOrder)[i] = i;
	}

	return circuit;
}



struct gateOrWire **readInCircuitRTL(char* filepath, int *numGates, int **execOrder)
{
	FILE *file = fopen ( filepath, "r" );
	char line [ 512 ]; // Or other suitable maximum line size
	struct gateOrWire *tempGateOrWire;
	struct gateOrWire **circuit;
	int numInputs1, numInputs2, numOutputs, gateIndex = 0;
	int i, execIndex;

	if ( file != NULL )
	{
		if(NULL != fgets(line, sizeof(line), file))
			sscanf(line, "%d %d", &numInputs1, numGates);
		else
			return NULL;

		if(NULL != fgets(line, sizeof(line), file))
			sscanf(line, "%d %d\t%d", &numInputs1, &numInputs2, &numOutputs);
		else
			return NULL;

		if(NULL == fgets(line, sizeof(line), file))
			return NULL;

		*execOrder = (int*) calloc(*numGates, sizeof(int));
		circuit = initialiseAllInputs(*numGates, numInputs1, numInputs2, execOrder);
		execIndex = numInputs1 + numInputs2;

		while ( fgets(line, sizeof(line), file) != NULL ) // Read a line
		{
			tempGateOrWire = processGateLineRTL(line, circuit);
			if( NULL != tempGateOrWire )
			{
				if(1 != tempGateOrWire -> gatePayload -> numInputs)
				{
					gateIndex = tempGateOrWire -> G_ID;
					*(circuit + gateIndex) = tempGateOrWire;
					(*execOrder)[execIndex++] = gateIndex;
				}
			}
		}

		for(i = 0; i < numOutputs; i ++)
		{
			gateIndex = *numGates - i - 1;
			circuit[gateIndex] -> outputWire -> wireMask = 0x0F;
		}
		fclose ( file );
	}

	return circuit;
}



/*
int isLineINV(char *line, int outputCutoff)
{
	int i = 0, idNum, ditchThisNumber, numInputs;


	numInputs = getIntFromString(line, strIndex);
	ditchThisNumber = getIntFromString(line, strIndex);
	for(i = 0; i < numInputs; i ++)
		ditchThisNumber = getIntFromString(line, strIndex);

	idNum = getIntFromString(line, strIndex);

	if(idNum < outputCutoff)
	{
		while('\0' != line[i])
		{
			if('I' == line[i])
				return 1;
		}
	}

	return 0;
}


int numberOfRemovedINVs(char* filepath, int *numGates)
{
	FILE *file = fopen ( filepath, "r" );
	char line [ 512 ];
	int numInputs1, numInputs2, numOutputs;
	int countINV = 0, i;

	if ( file != NULL )
	{
		if(NULL != fgets(line, sizeof(line), file))
			sscanf(line, "%d %d", &numInputs1, numGates);
		else
			return NULL;

		if(NULL != fgets(line, sizeof(line), file))
			sscanf(line, "%d %d\t%d", &numInputs1, &numInputs2, &numOutputs);
		else
			return NULL;

		if(NULL == fgets(line, sizeof(line), file))
			return NULL;


		// Read a line
		while ( fgets(line, sizeof(line), file) != NULL )
		{
			countINV += isLineINV(line, numGates - numOutputs);
		}

		fclose ( file );
	}

	return countINV;
}
*/