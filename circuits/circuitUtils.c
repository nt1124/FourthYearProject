// Run the circuit, executor style.
void runCircuitExec( struct Circuit *inputCircuit, int writeSocket, int readSocket )
{
	int i, gateID;

	for(i = 0; i < inputCircuit -> numGates; i ++)
	{
		gateID = inputCircuit -> execOrder[i];

		// Check there is a gate to be 
		if( NULL != inputCircuit -> gates[gateID] -> gatePayload )
		{
			evaulateGate(inputCircuit -> gates[gateID], inputCircuit -> gates);
		}
	}
}


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
		// printf("~~~ %d\n", i);
		// fflush(stdout);

		tempBit = inputCircuit -> gates[i] -> outputWire -> wirePermedValue;
		binaryOutput[j++] = tempBit ^ (0x01 & inputCircuit -> gates[i] -> outputWire -> wirePerm);
	}

	*binaryLength = j;

	return binaryOutput;
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


	// For every evaluation circuit
	for(i = 0; i < securityParam; i ++)
	{
		if(0x00 == J_set[i])
		{
			curBinaryStr = getOutputAsBinary(circuitsArray[i], &curLength);

			// If the circuits have different output lengths something is wrong.
			if(curLength != circuitsArray[0] -> numOutputs)
			{
				printf("Circuits have differing number of outputs.");
				fflush(stdout);

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


	// For each output wire we see which (0 or 1) is more common across all eval circuits.
	curBinaryStr = (unsigned char*) calloc(circuitsArray[0] -> numOutputs, sizeof(unsigned char));

	for(j = 0; j < curLength; j ++)
	{
		curBinaryStr[j] = (outputBitCounts[0][j] < outputBitCounts[1][j]);
	}

	*outputLength = curLength;


	free(outputBitCounts[0]);
	free(outputBitCounts[1]);
	free(outputBitCounts);

	return curBinaryStr;
}




// Prints all outputs as a binary string, taking gates in ascending position in the gates table.
void printMajorityOutputAsBinary(struct Circuit **circuitsArray, int securityParam, unsigned char *J_set)
{
	unsigned char *binaryOutput;
	int i, outputLength;


	if(1 < securityParam)
	{
		binaryOutput = majorityOutput(circuitsArray, securityParam, &outputLength, J_set);
	}
	else
	{
		binaryOutput = getOutputAsBinary(circuitsArray[0], &outputLength);
	}

	printf(" Majority output binary : ");
	for(i = 0; i < outputLength; i ++)
	{
		printf("%d", binaryOutput[i]);
	}
	printf("\n");

	free(binaryOutput);
}


unsigned char *getMajorityOutput(struct Circuit **circuitsArray, int securityParam, unsigned char *J_set)
{
	unsigned char *hexOutput, *binaryOutput;
	int outputLength;


	if(1 < securityParam)
	{
		binaryOutput = majorityOutput(circuitsArray, securityParam, &outputLength, J_set);
	}
	else
	{
		binaryOutput = getOutputAsBinary(circuitsArray[0], &outputLength);
	}

	return binaryOutput;
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
	int i, j, k = 0, numKeysToStore;
	int numInputsBuilder, numInputsExecutor;


	numInputsBuilder = circuitsArray[0] -> numInputsBuilder;
	numInputsExecutor = circuitsArray[0] -> numInputsExecutor;
	numKeysToStore = circuitsArray[0] -> numInputsExecutor * numCircuits;

	allKeys[0] = (unsigned char **) calloc(numKeysToStore, sizeof(unsigned char *));
	allKeys[1] = (unsigned char **) calloc(numKeysToStore, sizeof(unsigned char *));

	for(i = 0; i < numInputsBuilder + numInputsExecutor; i ++)
	{
		if(0xFF != circuitsArray[0] -> gates[i] -> outputWire -> wireOwner)
		{
			for(j = 0; j < numCircuits; j ++)
			{
				tempWire = circuitsArray[j] -> gates[i] -> outputWire;
				allKeys[0][k] = tempWire -> outputGarbleKeys -> key0;
				allKeys[1][k] = tempWire -> outputGarbleKeys -> key1;
				k ++;
			}
		}

	}

	return allKeys;
}


unsigned char *getPermedInputValuesExecutor(struct Circuit **circuitsArray, int partyID)
{
	int i, outputIndex = 0, numInputsBuilder, numInputsExecutor, gateIndex;
	unsigned char *output, value;


	numInputsBuilder = circuitsArray[0] -> numInputsBuilder;
	numInputsExecutor = circuitsArray[0] -> numInputsExecutor;
	output = (unsigned char *) calloc(numInputsExecutor, sizeof(unsigned char));

	for(i = 0; i < numInputsExecutor; i ++)
	{
		gateIndex = i + numInputsBuilder * (1 - partyID);
		value = circuitsArray[0] -> gates[gateIndex] -> outputWire -> wirePermedValue;
		output[outputIndex++] = value ^ (circuitsArray[0] -> gates[gateIndex] -> outputWire -> wirePerm & 0x01);
	}


	return output;
}


unsigned char *getPermedInputValuesExecutor(struct Circuit **circuitsArray)
{
	int i, outputIndex = 0, numInputsBuilder, numInputsExecutor;
	unsigned char *output, value;


	numInputsBuilder = circuitsArray[0] -> numInputsBuilder;
	numInputsExecutor = circuitsArray[0] -> numInputsExecutor;
	output = (unsigned char *) calloc(numInputsExecutor, sizeof(unsigned char));

	// For each input gate not owned by the builder, un-perm the wirePermedValue.
	for(i = 0; i < numInputsBuilder + numInputsExecutor; i ++)
	{
		if(0x00 == circuitsArray[0] -> gates[i] -> outputWire -> wireOwner)
		{
			value = circuitsArray[0] -> gates[i] -> outputWire -> wirePermedValue;
			output[outputIndex] = value ^ (circuitsArray[0] -> gates[i] -> outputWire -> wirePerm & 0x01);
			outputIndex ++;
		}
	}


	return output;
}