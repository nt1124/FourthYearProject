// Prints all outputs as a hex string, taking gates in ascending position in the gates table.
void outputAsHexString(struct Circuit *inputCircuit)
{
	int i, j = 0, k;
	unsigned char *binaryOutput, *hexOutput;
	unsigned char tempBit, tempHex;
	int numOutputs = 0;

	for(i = 0; i < inputCircuit -> numGates; i ++)
	{
		if( 0x02 == inputCircuit -> gates[i] -> outputWire -> wireMask )
			numOutputs ++;
	}

	binaryOutput = (unsigned char*) calloc(numOutputs, sizeof(unsigned char));

	for(i = 0; i < inputCircuit -> numGates; i ++)
	{
		if( 0x02 == inputCircuit -> gates[i] -> outputWire -> wireMask )
		{
			tempBit = inputCircuit -> gates[i] -> outputWire -> wirePermedValue;
			binaryOutput[j++] = tempBit ^ (0x01 & inputCircuit -> gates[i] -> outputWire -> wirePerm);
		}
	}

	hexOutput = (unsigned char*) calloc( (numOutputs / 8) + 2, sizeof(unsigned char) );

	j = 0;
	k = 7;
	tempHex = 0;
	for(i = 0; i < numOutputs; i ++)
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


// Run the circuit assuming we have the values for all input wires.
void runCircuitLocal( struct Circuit *inputCircuit)
{
	int i, j, gateID;

	for(i = 0; i < inputCircuit -> numGates; i ++)
	{
		gateID = inputCircuit -> execOrder[i];
		if( NULL != inputCircuit -> gates[gateID] -> gatePayload )
		{
			decryptGate(inputCircuit -> gates[gateID], inputCircuit -> gates);
		}
	}
}


// Ronseal.
struct Circuit *initBasicCircuit()
{
	struct Circuit *outputCircuit = (struct Circuit*) calloc(1, sizeof(struct Circuit));

	return outputCircuit;
}


// Ronseal.
void freeCircuitStruct(struct Circuit *toFree)
{
	int i;
    for(i = 0; i < toFree -> numGates; i ++)
    {
        freeGateOrWire(toFree -> gates[i]);
    }

    free(toFree -> gates);
    free(toFree);
}