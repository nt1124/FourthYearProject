// CnC means that this is building a Circuit with the encoding from Lindell/Pinkas Cut and Choose 2007.

// We assume party 1 is building the circuit.
struct gateOrWire **initialiseAllInputs_CnC(int numGates, int numInputs1, int numInputs2, int **execOrder, unsigned char *R, int securityParam)
{
	struct gateOrWire **circuit = (struct gateOrWire**) calloc(numGates, sizeof(struct gateOrWire*));
	int i, j, k, startIndex, index;
	int *inputIDs = (int*) calloc(2, sizeof(int));

	// printf("<<<<<   %d - %d - %d - %d\n", numGates, numInputs1, numInputs2, securityParam);

	for(i = 0; i < numInputs1; i ++)
	{
		circuit[i] = initialiseInputWire(i, 0xFF, R);
		(*execOrder)[i] = i;
	}

	for(i = numInputs1; i < numInputs1 + (numInputs2 * securityParam); i ++)
	{
		circuit[i] = initialiseInputWire(i, 0x00, R);
		(*execOrder)[i] = i;
	}

	index = numInputs1 + (numInputs2 * securityParam);
	for(i = 0; i < numInputs2; i ++)
	{
		inputIDs[0] = numInputs1 + (i * securityParam);
		inputIDs[1] = inputIDs[0] + 1;

		// printf("++++a   %d, %d, %d\n", index, inputIDs[0], inputIDs[1]);
		// fflush(stdout);

		circuit[index] = processGateOrWireRTL(index, inputIDs, 2, 'X', circuit,	R);
		(*execOrder)[index] = index;

		inputIDs[0] = inputIDs[1] + 1;
		inputIDs[1] = index;
		index ++;

		for(j = 1; j < (securityParam - 1); j ++)
		{
			// printf("+++++   %d - %d, %d\n", index, inputIDs[0], inputIDs[1]);
			// fflush(stdout);
			circuit[index] = processGateOrWireRTL(index, inputIDs, 2, 'X', circuit,	R);
			(*execOrder)[index] = index;


			inputIDs[0] += 1;
			inputIDs[1] += 1;

			index ++;
		}
	}

	return circuit;
}


// Create a circuit given a file in RTL format.
struct Circuit *readInCircuitRTL_CnC(char* filepath, int securityParam)
{
	if(1 < securityParam)
	{
		FILE *file = fopen ( filepath, "r" );
		char line [ 512 ]; // Or other suitable maximum line size
		int numInputs1, numInputs2, numOutputs, gateIndex = 0, ignore;

		struct gateOrWire *tempGateOrWire;
		struct gateOrWire **gatesList;

		struct Circuit *outputCircuit = (struct Circuit*) calloc(1, sizeof(struct Circuit));
		int i, execIndex, *execOrder, idOffset;
		unsigned char *R = generateRandBytes(16, 17);

		if ( file != NULL )
		{
			if(NULL != fgets(line, sizeof(line), file))
				sscanf( line, "%d %d", &ignore, &(outputCircuit -> numGates) );
			else
				return NULL;

			if(NULL != fgets(line, sizeof(line), file))
				sscanf(line, "%d %d\t%d", &numInputs1, &numInputs2, &(outputCircuit -> numOutputs));
			else
				return NULL;

			if(NULL == fgets(line, sizeof(line), file))
				return NULL;

			outputCircuit -> numInputsBuilder = numInputs1;
			outputCircuit -> numInputsExecutor = numInputs2;
			outputCircuit -> securityParam = securityParam;
			idOffset = 2 * numInputs2 * (securityParam - 1);

			// So we have enough room for all the extra gates/wires
			outputCircuit -> numGates += ( 2 * (securityParam - 1) * numInputs2 );
			outputCircuit -> numInputs = numInputs1 + numInputs2 * securityParam;

			execOrder = (int*) calloc(outputCircuit -> numGates, sizeof(int));
			gatesList = initialiseAllInputs_CnC( outputCircuit -> numGates, numInputs1, numInputs2, &execOrder, R, securityParam );
			execIndex = outputCircuit -> numInputs + numInputs2 * (securityParam - 1);

			while ( fgets(line, sizeof(line), file) != NULL ) // Read a line
			{
				tempGateOrWire = processGateLineRTL(line, gatesList, R, idOffset, numInputs1);
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
				gatesList[gateIndex] -> outputWire -> wireMask = 0x02;
			}

			fclose ( file );
		}


		outputCircuit -> gates = gatesList;
		outputCircuit -> execOrder = execOrder;

		free(R);

		return outputCircuit;
	}
	else
	{
		return readInCircuitRTL(filepath);
	}
}
