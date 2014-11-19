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
