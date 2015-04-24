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


// Function to .
struct RawGate *processGate_Raw(int idNum, int numInputWires, int *inputIDs, char gateType)
{
	struct RawGate *toReturn = (struct RawGate*) calloc(1, sizeof(struct RawGate));
	int i, outputTableSize = 1;


	toReturn -> G_ID = idNum;
	toReturn -> numInputs = numInputWires;
	toReturn -> inputIDs = (int*) calloc(numInputWires, sizeof(int));

	for(i = 0; i < numInputWires; i ++)
	{
		toReturn -> inputIDs[i] = inputIDs[i];
	}

	for(i = 0; i < toReturn -> numInputs; i ++)
		outputTableSize *= 2;

	toReturn -> outputTableSize = outputTableSize;
	toReturn -> gateType = gateType;
	if('I' == gateType)
	{
		toReturn -> rawOutputTable = (int*) calloc(2, sizeof(int));
		toReturn -> rawOutputTable[0] = 1;
	}
	else
	{
		toReturn -> rawOutputTable = (int*) calloc(4, sizeof(int));
		if('A' == gateType)
		{
			toReturn -> rawOutputTable[3] = 1;
		}
		else if('X' == gateType)
		{
			toReturn -> rawOutputTable[1] = 1;
			toReturn -> rawOutputTable[2] = 1;
		}
		else if('N' == gateType) // Inverted XOR
		{
			toReturn -> rawOutputTable[0] = 1;
			toReturn -> rawOutputTable[3] = 1;
		}
	}


	return toReturn;
}

// Take a line of the input file and make a gateOrWire struct from it.
struct RawGate *processGateLine_Raw(char *line, struct RawGate **circuit, int idOffset, int numInputs1)
{
	int strIndex = 0, idNum, i;
	int numInputWires, purposelessNumber;
	int *inputIDs;

	numInputWires = getIntFromString(line, strIndex);

	// As of yet unsure of the purpose of this number. Ask Nigel? Otherwise we just ignore it.
	purposelessNumber = getIntFromString(line, strIndex);

	inputIDs = (int*) calloc(numInputWires, sizeof(int));
	for(i = 0; i < numInputWires; i ++)
	{
		idNum = getIntFromString(line, strIndex);
		if(idNum >= numInputs1)
		{
			inputIDs[i] = idNum + idOffset;
		}
		else
		{
			inputIDs[i] = idNum;
		}
	}

	idNum = getIntFromString(line, strIndex) + idOffset;

	return processGate_Raw(idNum, numInputWires, inputIDs, line[strIndex]);
}


// Initialises an input wire, needed because input wires don't feature in the input file.
// Therefore they need to initialised separately.
struct RawGate *initialiseInputWire_Raw(int idNum)
{
	struct RawGate *toReturn = (struct RawGate*) calloc(1, sizeof(struct RawGate));

	toReturn -> G_ID = idNum;
	toReturn -> wireMask = 0x01; // N - stands for input...Yeah not great.

	return toReturn;
}


// We assume party 1 is building the circuit.
struct RawGate **initialiseAllInputs_Raw(int numGates, int numInputs1, int numInputs2, int **execOrder)
{
	struct RawGate **circuit = (struct RawGate**) calloc(numGates, sizeof(struct RawGate*));
	int i;

	for(i = 0; i < numInputs1; i ++)
	{
		circuit[i] = initialiseInputWire_Raw(i);
		circuit[i] -> outputValue = 0;
		(*execOrder)[i] = i;
	}

	for(; i < numInputs1 + numInputs2; i ++)
	{
		circuit[i] = initialiseInputWire_Raw(i);
		circuit[i] -> outputValue = 0;
		(*execOrder)[i] = i;
	}

	return circuit;
}


// Create a circuit given a file in RTL format.
struct RawCircuit *readInCircuit_Raw(char* filepath)
{
	FILE *file = fopen ( filepath, "r" );
	char line [ 512 ]; // Or other suitable maximum line size
	int numInputs1, numInputs2, numOutputs, gateIndex = 0;
	struct RawGate *tempGateOrWire;
	struct RawGate **gatesList;
	struct RawCircuit *outputCircuit = (struct RawCircuit*) calloc(1, sizeof(struct RawCircuit));
	int i, execIndex, *execOrder;


	if ( file != NULL )
	{
		if(NULL != fgets(line, sizeof(line), file))
			sscanf( line, "%d %d", &numInputs1, &(outputCircuit -> numGates) );
		else
			return NULL;

		if(NULL != fgets(line, sizeof(line), file))
			sscanf(line, "%d %d\t%d", &numInputs1, &numInputs2, &(outputCircuit -> numOutputs));
		else
			return NULL;

		if(NULL == fgets(line, sizeof(line), file))
			return NULL;


		outputCircuit -> numInputs_P1 = numInputs1;
		outputCircuit -> numInputs_P2 = numInputs2;
		outputCircuit -> numInputs = outputCircuit -> numInputs_P1 + outputCircuit -> numInputs_P2;

		execOrder = (int*) calloc(outputCircuit -> numGates, sizeof(int));
		gatesList = initialiseAllInputs_Raw( outputCircuit -> numGates, numInputs1, numInputs2, &execOrder);
		execIndex = numInputs1 + numInputs2;

		while ( fgets(line, sizeof(line), file) != NULL ) // Read a line
		{
			tempGateOrWire = processGateLine_Raw(line, gatesList, 0, numInputs1);
			if( NULL != tempGateOrWire )
			{
				gateIndex = tempGateOrWire -> G_ID;
				*(gatesList + gateIndex) = tempGateOrWire;
				*(execOrder + execIndex) = gateIndex;
				execIndex++;
			}
		}

		for(i = 0; i < outputCircuit -> numOutputs; i ++)
		{
			gateIndex = outputCircuit -> numGates - i - 1;
			gatesList[gateIndex] -> wireMask = 0x02;
		}
		fclose ( file );
	}

	outputCircuit -> gates = gatesList;
	outputCircuit -> execOrder = execOrder;


	return outputCircuit;
}


void evaluateRawGate(struct RawGate *curGate, struct RawGate **inputCircuit)
{
	int numInputs = curGate -> numInputs;
	int j, tempBit, tempIndex, outputIndex = 0;


	// For the all the input bits 
	for(j = 0; j < numInputs; j ++)
	{
		// Get the index of the j^th input wire, then get that wires permutated output value.
		tempIndex = curGate -> inputIDs[numInputs - j - 1];
		tempBit = inputCircuit[tempIndex] -> outputValue;

		// Get the next bit of the outputIndex.
		outputIndex = (outputIndex << 1) + tempBit;
	}

	// Get the decrypted permutated wire value.
	curGate -> outputValue = curGate -> rawOutputTable[outputIndex];
}


void evaluateRawCircuit(struct RawCircuit *inputCircuit)
{
	int i, gateID;

	for(i = 0; i < inputCircuit -> numGates; i ++)
	{
		gateID = inputCircuit -> execOrder[i];

		if(0x01 != inputCircuit -> gates[gateID] -> wireMask)
		{
			evaluateRawGate(inputCircuit -> gates[gateID], inputCircuit -> gates);
		}
	}
}


unsigned char *getOutputAsHex_Raw(struct RawCircuit *inputCircuit, int *outputLength)
{
	unsigned char *binaryOutput, *hexOutput;
	unsigned char tempBit, tempHex;
	int i, j = 0, k, iAdjusted, binaryLength = 0;


	hexOutput = (unsigned char*) calloc( (inputCircuit -> numOutputs / 8) + 2, sizeof(unsigned char) );

	j = 0;
	k = 7;
	tempHex = 0;
	for(i = 0; i < inputCircuit -> numOutputs; i ++)
	{
		iAdjusted = i + (inputCircuit -> numGates - inputCircuit -> numOutputs);
		hexOutput[j] += (inputCircuit -> gates[iAdjusted] -> outputValue << k);
		k --;

		if(7 == i % 8)
		{
			j++;
			k = 7;
		}
	}
	*outputLength = j;

	return hexOutput;
}


// Prints all outputs as a hex string, taking gates in ascending position in the gates table.
void printOutputHexString_Raw(struct RawCircuit *inputCircuit)
{
	unsigned char *hexOutput;
	int i, outputLength;
	
	hexOutput = getOutputAsHex_Raw(inputCircuit, &outputLength);

	printf("      Raw output as Hex: ");
	for(i = 0; i < outputLength; i ++)
	{
		printf("%02X", hexOutput[i]);
	}
	printf("\n");

	free(hexOutput);
}



void setRawCircuitsInputs_Hardcode(struct idAndValue *start, struct RawGate **gates)
{	
	struct idAndValue *current = start -> next;

	while(NULL != current)
	{
		gates[current -> id] -> outputValue = current -> value;
		current = current -> next;
	}
}

void setRawCircuitsInputs_Hardcode(unsigned char *inputs, int idOffset, int numInputs, struct RawGate **gates)
{	
	int i;

	for(i = 0; i < numInputs; i ++)
	{
		gates[i  + idOffset] -> outputValue = getBitFromCharArray(inputs, i);
	}
}


void freeRawGate(struct RawGate *toFree)
{
	int i;


	if(NULL != toFree)
	{
		if(NULL != toFree -> inputIDs)
		{
			free(toFree -> inputIDs);
			toFree -> inputIDs = NULL;
		}

		if(NULL != toFree -> rawOutputTable)
		{
			free(toFree -> rawOutputTable);
			toFree -> rawOutputTable = NULL;
		}

		free(toFree);
		toFree = NULL;
	}
}


void freeRawCircuit(struct RawCircuit *toFree)
{
	int i;

	if(NULL != toFree)
	{
		for(i = 0; i < toFree -> numGates; i ++)
		{
			freeRawGate(toFree -> gates[i]);
		}

		if(NULL != toFree -> gates)
		{
			free(toFree -> gates);
			toFree -> gates = NULL;
		}

		free(toFree);
	}
}