unsigned char *getOutputAsBinary(struct Circuit *inputCircuit, int *binaryLength)
{
	int i, j = 0, k;
	unsigned char *binaryOutput;
	unsigned char tempBit;
	int numOutputs = 0;

	numOutputs = inputCircuit -> numOutputs;

	binaryOutput = (unsigned char*) calloc(numOutputs, sizeof(unsigned char));

	for(i = inputCircuit -> numGates - inputCircuit -> numOutputs; i < inputCircuit -> numGates; i ++)
	{
		tempBit = inputCircuit -> gates[i] -> outputWire -> wirePermedValue;
		binaryOutput[j++] = tempBit ^ (0x01 & inputCircuit -> gates[i] -> outputWire -> wirePerm);
	}

	*binaryLength = j;

	return binaryOutput;
}

// unsigned char *getOutputAsHex(struct Circuit *inputCircuit, int *outputLength)
unsigned char *getOutputAsHex(unsigned char *binaryStr, int numOutputs, int *outputLength)
{
	unsigned char *binaryOutput, *hexOutput;
	unsigned char tempBit, tempHex;
	int i, j = 0, k, binaryLength = 0;


	//binaryOutput = getOutputAsBinary(inputCircuit, &binaryLength);

	hexOutput = (unsigned char*) calloc( (numOutputs / 8) + 2, sizeof(unsigned char) );

	j = 0;
	k = 7;
	tempHex = 0;
	for(i = 0; i < numOutputs; i ++)
	{
		hexOutput[j] += (binaryStr[i] << k);
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
void printOutputHexString(struct Circuit *inputCircuit)
{
	unsigned char *binaryOutput;
	unsigned char *hexOutput;
	int i, outputLength;
	
	binaryOutput = getOutputAsBinary(inputCircuit, &outputLength);
	hexOutput = getOutputAsHex(binaryOutput, inputCircuit -> numOutputs, &outputLength);

	printf("Candidate output as Hex: ");
	for(i = 0; i < outputLength; i ++)
	{
		printf("%02X", hexOutput[i]);
	}
	printf("\n");

	free(binaryOutput);
	free(hexOutput);
}


// Prints the value of all wires that have an output flag raised.
void printAllOutput(struct Circuit *inputCircuit)
{
	int i;
	unsigned char tempBit;

	for(i = 0; i < inputCircuit -> numGates; i ++)
	{
		if( 0x02 == (0x0F & inputCircuit -> gates[i] -> outputWire -> wireMask) )
		{
			tempBit = inputCircuit -> gates[i] -> outputWire -> wirePermedValue;
			tempBit = tempBit ^ (0x01 & inputCircuit -> gates[i] -> outputWire -> wirePerm);
			printf("Gate %d = %d\n", inputCircuit -> gates[i] -> G_ID, tempBit);
		}
	}
}


// Get the output from a set of Circuits 
unsigned char *majorityOutput(struct Circuit **circuitsArray, int securityParam, int *outputLength, unsigned char *J_set)
{
	unsigned char *finalOutput;
	unsigned char *curBinaryStr;
	int **outputBitCounts = (int**) calloc(2, sizeof(int*));
	int i, j, count = 0, curLength;

	int k;

	outputBitCounts[0] = (int*) calloc(circuitsArray[0] -> numOutputs, sizeof(int));
	outputBitCounts[1] = (int*) calloc(circuitsArray[0] -> numOutputs, sizeof(int));

	for(i = 0; i < securityParam; i ++)
	{
		if(0x00 == J_set[i])
		{
			curBinaryStr = getOutputAsBinary(circuitsArray[i], &curLength);

			if(curLength != circuitsArray[0] -> numOutputs)
			{
				printf("Circuits have differing number of outputs.");
				free(outputBitCounts[0]);
				free(outputBitCounts[1]);
				free(outputBitCounts);
				free(curBinaryStr);
				return NULL;
			}

			for(j = 0; j < curLength; j ++)
			{
				outputBitCounts[curBinaryStr[j]][j] ++; 
			}
			free(curBinaryStr);
		}
	}

	curBinaryStr = (unsigned char*) calloc(circuitsArray[0] -> numOutputs, sizeof(unsigned char));

	for(j = 0; j < curLength; j ++)
	{
		curBinaryStr[j] = (outputBitCounts[0][j] < outputBitCounts[1][j]);
	}

	finalOutput = getOutputAsHex(curBinaryStr, curLength, outputLength);

	free(outputBitCounts[0]);
	free(outputBitCounts[1]);
	free(outputBitCounts);

	return finalOutput;
}


// Prints all outputs as a hex string, taking gates in ascending position in the gates table.
void printMajorityOutputAsHex(struct Circuit **circuitsArray, int securityParam, unsigned char *J_set)
{
	unsigned char *hexOutput;
	int i, outputLength;
	
	if(1 < securityParam)
	{
		hexOutput = majorityOutput(circuitsArray, securityParam, &outputLength, J_set);
	}
	else
	{
		unsigned char *binaryOutput = getOutputAsBinary(circuitsArray[0], &outputLength);
		hexOutput = getOutputAsHex(binaryOutput, circuitsArray[0] -> numOutputs, &outputLength);
	}

	printf(" Majority output as Hex: ");
	for(i = 0; i < outputLength; i ++)
	{
		printf("%02X", hexOutput[i]);
	}
	printf("\n");

	free(hexOutput);
}


// Run the circuit assuming we have the values for all input wires.
void runCircuitLocal( struct Circuit *inputCircuit)
{
	int i, j, gateID;
	struct wire *temp;

	for(i = 0; i < inputCircuit -> numGates; i ++)
	{
		gateID = inputCircuit -> execOrder[i];
		if( NULL != inputCircuit -> gates[gateID] -> gatePayload )
		{
			evaulateGate(inputCircuit -> gates[gateID], inputCircuit -> gates);
		}
		temp = inputCircuit -> gates[gateID] -> outputWire;
	}
}



// Ronseal.
struct Circuit *initBasicCircuit()
{
	struct Circuit *outputCircuit = (struct Circuit*) calloc(1, sizeof(struct Circuit));

	return outputCircuit;
}


// Ronseal.
void freeCircuitStruct(struct Circuit *toFree, int freeExecList)
{
	int i;


	for(i = 0; i < toFree -> numGates; i ++)
	{
		freeGateOrWire(toFree -> gates[i]);
	}

	free(toFree -> gates);

	if(1 == freeExecList)
	{
		free(toFree -> execOrder);
	}

	free(toFree);
}



void freeTempGarbleCircuit(struct Circuit *tempGarbleCircuit)
{
	int i, j;

	for(i = 0; i < tempGarbleCircuit -> numGates; i ++)
	{
		if(NULL != tempGarbleCircuit -> gates[i] -> gatePayload)
		{
			if(NULL != tempGarbleCircuit -> gates[i] -> gatePayload -> encOutputTable)
			{
				for(j = 0; j < tempGarbleCircuit -> gates[i] -> gatePayload -> outputTableSize; j ++)
				{
					free(tempGarbleCircuit -> gates[i] -> gatePayload -> encOutputTable[j]);
				}
				free(tempGarbleCircuit -> gates[i] -> gatePayload -> encOutputTable);
			}
			free(tempGarbleCircuit -> gates[i] -> gatePayload);
		}

		free(tempGarbleCircuit -> gates[i] -> outputWire -> outputGarbleKeys -> key0);
		free(tempGarbleCircuit -> gates[i] -> outputWire -> outputGarbleKeys -> key1);
		free(tempGarbleCircuit -> gates[i] -> outputWire -> outputGarbleKeys);
		free(tempGarbleCircuit -> gates[i] -> outputWire -> wireOutputKey);
		free(tempGarbleCircuit -> gates[i] -> outputWire);

		free(tempGarbleCircuit -> gates[i]);
	}

	free(tempGarbleCircuit -> gates);

	free(tempGarbleCircuit);
}


// Function that gets the OT inputs as an array from the circuits.
unsigned char ***getAllInputKeys(struct Circuit **circuitsArray, int numCircuits)
{
	unsigned char ***allKeys = (unsigned char ***) calloc(2, sizeof(unsigned char **));
	struct wire *tempWire;
	int i, j, k, numKeysToStore;
	int numInputsBuilder, numInputsExecutor;


	numInputsBuilder = circuitsArray[0] -> numInputsBuilder;
	numInputsExecutor = circuitsArray[0] -> numInputsExecutor;
	numKeysToStore = circuitsArray[0] -> numInputsExecutor * numCircuits;

	allKeys[0] = (unsigned char **) calloc(numKeysToStore, sizeof(unsigned char *));
	allKeys[1] = (unsigned char **) calloc(numKeysToStore, sizeof(unsigned char *));

	for(i = numInputsBuilder; i < numInputsBuilder + numInputsExecutor; i ++)
	{
		for(j = 0; j < numCircuits; j ++)
		{
			tempWire = circuitsArray[j] -> gates[i] -> outputWire;
			allKeys[0][k] = tempWire -> outputGarbleKeys -> key0;
			allKeys[1][k] = tempWire -> outputGarbleKeys -> key1;

			k ++;
		}

	}

	return allKeys;
}


unsigned char *getPermedInputValuesExecutor(struct Circuit **circuitsArray)
{
	int i, outputIndex = 0, numInputsBuilder, numInputsExecutor;
	unsigned char *output, value;


	numInputsBuilder = circuitsArray[0] -> numInputsBuilder;
	numInputsExecutor = circuitsArray[0] -> numInputsExecutor;
	output = (unsigned char *) calloc(numInputsExecutor, sizeof(unsigned char));

	for(i = numInputsBuilder; i < numInputsBuilder + numInputsExecutor; i ++)
	{
		value = circuitsArray[0] -> gates[i] -> outputWire -> wirePermedValue;
		output[outputIndex++] = value ^ (circuitsArray[0] -> gates[i] -> outputWire -> wirePerm & 0x01);
	}


	return output;
}