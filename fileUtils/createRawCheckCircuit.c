// Create a circuit given a file in RTL format.
struct RawCircuit *createRawCheckCircuit(int inputB_Size, int numBitsToCheck)
{
	int numInputs1, numInputs2, gateIndex = 0;
	struct RawGate *tempGateOrWire;
	struct RawGate **gatesList;
	struct RawCircuit *outputCircuit = (struct RawCircuit*) calloc(1, sizeof(struct RawCircuit));
	int i, execIndex, *execOrder, *inputIDs;
	char N_Char = 'N',  MegaAND_Char = 'B';


	// numGates = Builder inputs + Delta inputs + NXORs on Deltas + Mega AND + x_i AND compCheck.
	outputCircuit -> numGates = 2 * inputB_Size + 3 * numBitsToCheck + 1;

	// Set the number of inputs for each party and the number of outputs
	outputCircuit -> numInputsBuilder = numBitsToCheck + inputB_Size;
	outputCircuit -> numInputsExecutor = numBitsToCheck;
	outputCircuit -> numOutputs = inputB_Size; 

	// Initilise the inputs and stuff.
	execOrder = (int*) calloc(outputCircuit -> numGates, sizeof(int));
	gatesList = initialiseAllInputs_Raw( outputCircuit -> numGates, outputCircuit -> numInputsBuilder, outputCircuit -> numInputsExecutor, &execOrder);
	gateIndex = execIndex = 2 * numBitsToCheck + inputB_Size;

	inputIDs = (int*) calloc(2, sizeof(int));
	inputIDs[0] = 0;
	inputIDs[1] = numBitsToCheck + inputB_Size;
	for(i = 0; i < numBitsToCheck; i ++)
	{
		tempGateOrWire = processGate_Raw(execIndex, 2, inputIDs, 'N');

		if( NULL != tempGateOrWire )
		{
			gateIndex = tempGateOrWire -> G_ID;
			*(gatesList + gateIndex) = tempGateOrWire;
			*(execOrder + execIndex) = gateIndex;

			inputIDs[0] ++;
			inputIDs[1] ++;
			execIndex ++;
		}
	}

	// The Mega AND gate. We use M to denote this.
	// In the case of the MegaAND gate the inputs IDs represent an INCLUSIVE Range.
	inputIDs[0] = 2 * numBitsToCheck + inputB_Size;
	inputIDs[1] = execIndex - 1;
	*(gatesList + gateIndex) = processGate_Raw(execIndex, 2, inputIDs, 'M');
	*(execOrder + execIndex) = gateIndex;
	execIndex ++;

	inputIDs[0] = 0;
	inputIDs[1] = 3 * numBitsToCheck + inputB_Size;
	for(i = 0; i < outputCircuit -> numOutputs; i ++)
	{
		tempGateOrWire = processGate_Raw(execIndex, 2, inputIDs, 'A');

		if( NULL != tempGateOrWire )
		{
			gateIndex = tempGateOrWire -> G_ID;
			*(gatesList + gateIndex) = tempGateOrWire;
			*(execOrder + execIndex) = gateIndex;

			inputIDs[0] ++;
			execIndex ++;
		}
	}

	for(i = 0; i < outputCircuit -> numOutputs; i ++)
	{
		gateIndex = outputCircuit -> numGates - i - 1;
		gatesList[gateIndex] -> wireMask = 0x02;
	}


	outputCircuit -> gates = gatesList;
	outputCircuit -> execOrder = execOrder;


	return outputCircuit;
}
