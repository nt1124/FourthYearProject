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


void setDeltaXOR_onCircuitInputs(struct Circuit **circuitsArray, unsigned char **OT_Outputs, unsigned char *delta,
								int lengthDelta, int checkStatSecParam)
{
	int i, j, k, iOffset, OT_index, u_v_index;
	unsigned char **XORedInputs = (unsigned char **) calloc(checkStatSecParam, sizeof(unsigned char *));


	for(j = 0; j < checkStatSecParam; j ++)
	{
		XORedInputs[j] = (unsigned char *) calloc(16, sizeof(unsigned char));
	}

	for(i = 0; i < lengthDelta; i ++)
	{
		iOffset = checkStatSecParam * i;
		u_v_index = 2 * iOffset;

		for(j = 0; j < checkStatSecParam; j ++)
		{
			if(NULL != OT_Outputs[u_v_index + delta[i]])
			{
				for(k = 0; k < 16; k ++)
				{
					XORedInputs[j][k] ^= OT_Outputs[u_v_index + delta[i]][k];
				}
			}
			else
			{
				printf(">>>>>>>>>>>>>>>>>>>>>\n");
			}

			u_v_index += 2;
		}
	}

	for(j = 0; j < checkStatSecParam; j ++)
	{
		printf("\n%03d --  ", j);
		for(k = 0; k < 16; k ++)
		{
			printf("%02X", XORedInputs[j][k]);
		}
	}
	printf("\n");
}


unsigned char *SC_DetectCheatingExecutor(int writeSocket, int readSocket, struct RawCircuit *rawInputCircuit,
										unsigned char *deltaPrime, int lengthDelta,
										int checkStatSecParam, gmp_randstate_t *state )
{
	struct Circuit **circuitsArray = (struct Circuit **) calloc(checkStatSecParam, sizeof(struct Circuit*));
	struct publicInputsWithGroup *pubInputGroup;
	struct eccParams *params;
	struct idAndValue *startOfInputChain = convertArrayToChain(deltaPrime, lengthDelta, 0);

	unsigned char *commBuffer, *J_set, **OT_Outputs, *output;
	unsigned int *seedList;
	int commBufferLen = 0, i;


	params = initBrainpool_256_Curve();
	pubInputGroup = receivePublicCommitments(writeSocket, readSocket);

	for(i = 0; i < checkStatSecParam; i ++)
	{
		circuitsArray[i] = receiveFullCircuit(writeSocket, readSocket);
	}


	// OT_Inputs
	printf("EXECUTOR IS RECEIVING THE OTs\n");
	deltaPrime = (unsigned char *) calloc(lengthDelta, sizeof(unsigned char));
	J_set = full_CnC_OT_Receiver_ECC_Alt(writeSocket, readSocket, lengthDelta,
										state, deltaPrime, &OT_Outputs, checkStatSecParam, 1024);
	setDeltaXOR_onCircuitInputs(circuitsArray, OT_Outputs, deltaPrime, lengthDelta, checkStatSecParam);

	for(i = 0; i < checkStatSecParam; i ++)
	{
		freeCircuitStruct(circuitsArray[i], 0);
	}

	return output;
}

