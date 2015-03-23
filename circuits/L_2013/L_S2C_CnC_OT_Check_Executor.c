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


unsigned char *deserialiseK0sAndDelta(unsigned char *commBuffer, struct Circuit **circuitsArray, int numInputsB, int checkStatSecParam)
{
	struct wire *tempWire;
	unsigned char *delta = (unsigned char *) calloc(128, sizeof(unsigned char));
	int i, bufferOffset = 0;
	
	for(i = 0; i < checkStatSecParam; i ++)
	{
		tempWire = circuitsArray[i] -> gates[numInputsB] -> outputWire;

		tempWire -> outputGarbleKeys = (struct bitsGarbleKeys *) calloc(1, sizeof(struct bitsGarbleKeys));
		tempWire -> outputGarbleKeys -> key0 = (unsigned char *) calloc(16, sizeof(unsigned char));
		tempWire -> outputGarbleKeys -> key1 = (unsigned char *) calloc(16, sizeof(unsigned char));

		memcpy(tempWire -> outputGarbleKeys -> key0, commBuffer + bufferOffset, 16);
		bufferOffset += 16;
	}

	memcpy(delta, commBuffer + bufferOffset, 128);
	bufferOffset += 128;

	return delta;
}


void setDeltaXOR_onCircuitInputs(struct Circuit **circuitsArray, unsigned char **OT_Outputs,
								unsigned char *delta, unsigned char *deltaPrime, unsigned char *J_set,
								int numInputsB, int lengthDelta, int checkStatSecParam)
{
	struct wire *tempWire;
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
			if(0x00 == J_set[j] && NULL != OT_Outputs[u_v_index + deltaPrime[i]])
			{
				for(k = 0; k < 16; k ++)
				{
					XORedInputs[j][k] ^= OT_Outputs[u_v_index + deltaPrime[i]][k];
				}
			}
			else if(NULL != OT_Outputs[u_v_index + delta[i]])
			{
				for(k = 0; k < 16; k ++)
				{
					XORedInputs[j][k] ^= OT_Outputs[u_v_index + delta[i]][k];
				}
			}
			u_v_index += 2;
		}
	}

	for(j = 0; j < checkStatSecParam; j ++)
	{
		printf(">> %d  (0)\n", j);
		fflush(stdout);
		tempWire = circuitsArray[j] -> gates[numInputsB] -> outputWire;
		printf(">> %d  (1)\n", j);
		fflush(stdout);
		memcpy(tempWire -> outputGarbleKeys -> key1, XORedInputs[j], 16);
		printf(">> %d  (2)\n", j);
		fflush(stdout);
	}
}


unsigned char *SC_DetectCheatingExecutor(int writeSocket, int readSocket, struct RawCircuit *rawInputCircuit,
										unsigned char *deltaPrime, int lengthDelta,
										int checkStatSecParam, gmp_randstate_t *state )
{
	struct Circuit **circuitsArray = (struct Circuit **) calloc(checkStatSecParam, sizeof(struct Circuit*));
	struct publicInputsWithGroup *pubInputGroup;
	struct eccParams *params;
	struct idAndValue *startOfInputChain = convertArrayToChain(deltaPrime, lengthDelta, 0);

	unsigned char *commBuffer, *J_set, **OT_Outputs, *output, *delta;
	unsigned int *seedList;
	int commBufferLen = 0, i;

	struct timespec int_t_0, int_t_1;
	clock_t int_c_0, int_c_1;


	params = initBrainpool_256_Curve();
	pubInputGroup = receivePublicCommitments(writeSocket, readSocket);

	for(i = 0; i < checkStatSecParam; i ++)
	{
		circuitsArray[i] = receiveFullCircuit(writeSocket, readSocket);
	}


	// OT_Inputs
	printf("EXECUTOR IS RECEIVING THE OTs\n");
	int_t_0 = timestamp();
	int_c_0 = clock();

	J_set = full_CnC_OT_Receiver_ECC_Alt(writeSocket, readSocket, lengthDelta,
										state, deltaPrime, &OT_Outputs, checkStatSecParam, 1024);

	commBuffer = receiveBoth(readSocket, commBufferLen);
	delta = deserialiseK0sAndDelta(commBuffer, circuitsArray, rawInputCircuit -> numInputsBuilder, checkStatSecParam);


	setDeltaXOR_onCircuitInputs(circuitsArray, OT_Outputs, delta, deltaPrime, J_set,
								rawInputCircuit -> numInputsBuilder, lengthDelta, checkStatSecParam);

	int_c_1 = clock();
	int_t_1 = timestamp();
	printTiming(&int_t_0, &int_t_1, int_c_0, int_c_1, "subOT - Receiver");

	for(i = 0; i < checkStatSecParam; i ++)
	{
		freeCircuitStruct(circuitsArray[i], 0);
	}

	return output;
}

