// Create a circuit given a file in RTL format.
struct RawCircuit *createRawCheckCircuit(int inputB_Size)
{
	int numInputs1, numInputs2, gateIndex = 0;
	struct RawGate *tempGateOrWire;
	struct RawGate **gatesList;
	struct RawCircuit *outputCircuit = (struct RawCircuit*) calloc(1, sizeof(struct RawCircuit));
	int i, execIndex, *execOrder, *inputIDs;


	// numGates = Builder inputs + x_i AND k_1 + inputWire for Executor.
	outputCircuit -> numGates = 2 * inputB_Size + 1;

	// Set the number of inputs for each party and the number of outputs
	outputCircuit -> numInputs_P1 = inputB_Size;
	outputCircuit -> numInputs_P2 = 1;
	outputCircuit -> numOutputs = inputB_Size; 

	// Initilise the inputs and stuff.
	execOrder = (int*) calloc(outputCircuit -> numGates, sizeof(int));
	gatesList = initialiseAllInputs_Raw( outputCircuit -> numGates, outputCircuit -> numInputs_P1,
										outputCircuit -> numInputs_P2, &execOrder);
	execIndex = inputB_Size + 1;

	inputIDs = (int*) calloc(2, sizeof(int));
	inputIDs[0] = 0;
	inputIDs[1] = inputB_Size;
	for(i = 0; i < inputB_Size; i ++)
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
