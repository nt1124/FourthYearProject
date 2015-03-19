unsigned char *getDeltaPrimeCompressed(struct Circuit **circuitsArray, unsigned char *J_set, int stat_SecParams)
{
	unsigned char *deltaPrime;
	struct wire *zeroCircuitWire, *iCircuitWire;
	int i, j, k, firstJ = 0;
	const int iOffset = circuitsArray[0] -> numGates - circuitsArray[0] -> numOutputs;


	while(J_set[firstJ] == 0x01)
	{
		firstJ ++;
	}

	for(i = 0; i < circuitsArray[0] -> numOutputs; i ++)
	{
		zeroCircuitWire = circuitsArray[firstJ] -> gates[i + iOffset] -> outputWire;
		for(j = firstJ + 1; j < stat_SecParams; j ++)
		{

			if( 0x00 == J_set[j])
			{
				iCircuitWire = circuitsArray[j] -> gates[i + iOffset] -> outputWire;
				if(0 != memcmp(zeroCircuitWire -> wireOutputKey, iCircuitWire -> wireOutputKey, 16))
				{
					deltaPrime = (unsigned char *) calloc(16, sizeof(unsigned char));
					for(k = 0; k < 16; k ++)
					{
						deltaPrime[k] = zeroCircuitWire -> wireOutputKey[k] ^ iCircuitWire -> wireOutputKey[k];
					}

					printf("\nAha! Delta Prime triggered!  %d\n", j);

					return deltaPrime;
				}
			}
		}
	}

	return generateRandBytes(16, 16);
}


unsigned char *expandDeltaPrim(struct Circuit **circuitsArray, unsigned char *J_set, int checkStatSecParam)
{
	unsigned char *deltaPrimeComp, *deltaPrime = (unsigned char *) calloc(128, sizeof(unsigned char));
	int i;

	deltaPrimeComp = getDeltaPrimeCompressed(circuitsArray, J_set, checkStatSecParam);

	for(i = 0; i < 128; i ++)
	{
		deltaPrime[i] = getBitFromCharArray(deltaPrimeComp, i);
	}

	return deltaPrime;
}


unsigned char *SC_DetectCheatingExecutor(int writeSocket, int readSocket, struct RawCircuit *rawInputCircuit,
										unsigned char *deltaPrime, int lengthDelta,
										int checkStatSecParam, gmp_randstate_t *state )
{
	struct Circuit **circuitsArray = (struct Circuit **) calloc(checkStatSecParam, sizeof(struct Circuit*));
	struct publicInputsWithGroup *pubInputGroup;
	struct eccParams *params;
	struct idAndValue *startOfInputChain = convertArrayToChain(deltaPrime, lengthDelta, 0);

	unsigned char *commBuffer, *J_set, ***OT_Inputs, *output;
	unsigned int *seedList;
	int commBufferLen = 0, i;


	params = initBrainpool_256_Curve();
	pubInputGroup = receivePublicCommitments(writeSocket, readSocket);

	for(i = 0; i < checkStatSecParam; i ++)
	{
		circuitsArray[i] = receiveFullCircuit(writeSocket, readSocket);
	}



	for(i = 0; i < checkStatSecParam; i ++)
	{
		freeCircuitStruct(circuitsArray[i], 0);
	}

	return output;
}

