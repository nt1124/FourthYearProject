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



// Create a circuit to take two b-bit inputs and one l-bit input.
// Check if two b-bit inputs are the same, if so output the l-bit input.
struct RawCircuit *createRawCheckCircuit_No_OT_Opt(int inputX_Size, int bitsToCheck)
{
	int numInputs1, numInputs2, gateIndex = 0;
	struct RawGate *tempGateOrWire;
	struct RawGate **gatesList;
	struct RawCircuit *outputCircuit = (struct RawCircuit*) calloc(1, sizeof(struct RawCircuit));
	int i, execIndex, *execOrder, *inputIDs, tempIndex;

	// xInput + delta + deltaPrime + NXORs on deltas + ANDs on NXORs + ANDs on ANDs and xInput
	outputCircuit -> numGates = inputX_Size + (3 * bitsToCheck) + (bitsToCheck - 1) + inputX_Size;

	// Set the number of inputs for each party and the number of outputs
	outputCircuit -> numInputs_P1 = inputX_Size + bitsToCheck;
	outputCircuit -> numInputs_P2 = bitsToCheck;
	outputCircuit -> numOutputs = inputX_Size;

	// Initilise the inputs and stuff.
	execOrder = (int*) calloc(outputCircuit -> numGates, sizeof(int));
	gatesList = initialiseAllInputs_Raw(outputCircuit -> numGates, outputCircuit -> numInputs_P1,
										outputCircuit -> numInputs_P2, &execOrder);

	execIndex = outputCircuit -> numInputs_P1 + outputCircuit -> numInputs_P2;
	tempIndex = execIndex;

	inputIDs = (int*) calloc(2, sizeof(int));
	inputIDs[0] = inputX_Size;
	inputIDs[1] = outputCircuit -> numInputs_P1;

	for(i = 0; i < bitsToCheck; i ++)
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

	inputIDs[0] = tempIndex;
	inputIDs[1] = tempIndex + 1;
	tempGateOrWire = processGate_Raw(execIndex, 2, inputIDs, 'A');
	gateIndex = tempGateOrWire -> G_ID;
	*(gatesList + gateIndex) = tempGateOrWire;
	*(execOrder + execIndex) = gateIndex;

	inputIDs[0] = execIndex;
	inputIDs[1] = tempIndex + 2;
	execIndex ++;


	for(i = 0; i < bitsToCheck - 2; i ++)
	{
		tempGateOrWire = processGate_Raw(execIndex, 2, inputIDs, 'A');

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

	inputIDs[0] = 0;
	inputIDs[1] = execIndex - 1;
	for(i = 0; i < inputX_Size; i ++)
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



void testRawCheckCircuits()
{
	unsigned char *temp;
	struct RawCircuit *circuitTime;
	int i, outputLength = 0;

	circuitTime = createRawCheckCircuit_No_OT_Opt(128, 40);

	circuitTime -> gates[0] -> outputValue = 1;
	circuitTime -> gates[1] -> outputValue = 1;
	circuitTime -> gates[2] -> outputValue = 1;
	circuitTime -> gates[3] -> outputValue = 1;

	circuitTime -> gates[100] -> outputValue = 1;
	circuitTime -> gates[121] -> outputValue = 1;

	circuitTime -> gates[129] -> outputValue = 1;
	circuitTime -> gates[169] -> outputValue = 1;

	evaluateRawCircuit(circuitTime);

	temp = getOutputAsHex_Raw(circuitTime, &outputLength);

	for(i = 0; i < outputLength; i ++)
	{
		printf("%02X", temp[i]);
	}
	printf("\n");
}





