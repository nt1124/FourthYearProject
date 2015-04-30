struct Circuit **buildAllCircuitsConsistentOutput(struct RawCircuit *rawInputCircuit, struct idAndValue *startOfInputChain,
												struct eccPoint ***NP_consistentInputs, unsigned char **b0List, unsigned char **b1List,
												struct eccParams *params, randctx **circuitCTXs, int numCircuits)
{
	struct Circuit **circuitsArray = (struct Circuit **) calloc(numCircuits, sizeof(struct Circuit*));
	struct idAndValue *start;
	int j;


	#pragma omp parallel for private(j) schedule(auto)
	for(j = 0; j < numCircuits; j++)
	{
		circuitsArray[j] = readInCircuit_FromRaw_Seeded_ConsistentInputOutput(circuitCTXs[j], rawInputCircuit, NP_consistentInputs[j],
																			b0List, b1List, j, params);
	}


	for(j = 0; j < numCircuits; j++)
	{
		start = startOfInputChain;
		setCircuitsInputs_Hardcode(start, circuitsArray[j], 0xFF);
	}


	return circuitsArray;
}


unsigned char *jSetRevealSerialise_L_2013_HKE(struct eccPoint ***NaorPinkasInputs, mpz_t **aList, unsigned char *inputBitsOwn,
											ub4 **circuitSeeds,	unsigned char *jSet, int numInputs, int numCircuits, int *outputLength)
{
	unsigned char *outputBuffer;
	int i, j, k, tempOffset = 0;
	int aListLen = 0, seedLengths = 0;

	(*outputLength) = 0;

	for(j = 0; j < numCircuits; j ++)
	{
		if(0x01 == jSet[j])
		{
			k = 0;

			for(i = 0; i < numInputs; i ++)
			{
				aListLen += sizeOfSerialMPZ(aList[j][k]);
				k ++;

				aListLen += sizeOfSerialMPZ(aList[j][k]);
				k ++;
			}
			seedLengths += (256 * sizeof(ub4));
		}
		else
		{
			for(i = 0; i < numInputs; i ++)
			{
				if(0x00 == inputBitsOwn[i])
				{
					(*outputLength) += sizeOfSerial_ECCPoint(NaorPinkasInputs[j][2*i]);
				}
				else
				{
					(*outputLength) += sizeOfSerial_ECCPoint(NaorPinkasInputs[j][2*i + 1]);
				}
			}
		}
	}

	(*outputLength) += (aListLen + seedLengths);
	outputBuffer = (unsigned char *) calloc(*outputLength, sizeof(unsigned char));

	for(j = 0; j < numCircuits; j ++)
	{
		if(0x01 == jSet[j])
		{
			k = 0;

			for(i = 0; i < numInputs; i ++)
			{
				serialiseMPZ(aList[j][k], outputBuffer, &tempOffset);
				k ++;

				serialiseMPZ(aList[j][k], outputBuffer, &tempOffset);
				k ++;
			}

			memcpy(outputBuffer + tempOffset, circuitSeeds[j], 256 * sizeof(ub4));
			tempOffset += 256 * sizeof(ub4);
		}
	}


	for(j = 0; j < numCircuits; j ++)
	{
		if(0x00 == jSet[j])
		{
			for(i = 0; i < numInputs; i ++)
			{
				if(0x00 == inputBitsOwn[i])
				{
					serialise_ECC_Point(NaorPinkasInputs[j][2*i], outputBuffer, &tempOffset);
				}
				else
				{
					serialise_ECC_Point(NaorPinkasInputs[j][2*i + 1], outputBuffer, &tempOffset);
				}
			}
		}
	}

	return outputBuffer;
}


unsigned char *builder_decommitToJ_Set_L_2013_HKE(int writeSocket, int readSocket, struct Circuit **circuitsArray,
												struct eccPoint ***NP_consistentInputs, mpz_t **aList, unsigned char *inputBitsOwn,
												int stat_SecParam, int *J_setSize, ub4 **circuitSeeds)
{
	struct wire *tempWire;
	unsigned char *commBuffer, *J_Set;
	int tempOffset = stat_SecParam; 
	unsigned char key0_Correct, key1_Correct, finalOutput = 0x00;
	int i, count = 0, commBufferLen = 0;


	J_Set = (unsigned char *) calloc(stat_SecParam, sizeof(unsigned char));

	commBuffer = receiveBoth(readSocket, commBufferLen);
	memcpy(J_Set, commBuffer, stat_SecParam);


	for(i = 0; i < stat_SecParam; i ++)
	{
		if(0x01 == J_Set[i])
		{
			tempWire = circuitsArray[i] -> gates[circuitsArray[i] -> numInputsBuilder] -> outputWire;

			key0_Correct = memcmp(commBuffer + tempOffset, tempWire -> outputGarbleKeys -> key0, 16);
			tempOffset += 16;

			key1_Correct = memcmp(commBuffer + tempOffset, tempWire -> outputGarbleKeys -> key1, 16);
			tempOffset += 16;

			finalOutput = finalOutput | key0_Correct | key1_Correct;
			count ++;
		}
	}
	free(commBuffer);


	if(0x00 == finalOutput)
	{
		commBufferLen = 0;
		commBuffer = jSetRevealSerialise_L_2013_HKE(NP_consistentInputs, aList, inputBitsOwn, circuitSeeds, J_Set,
													circuitsArray[0] -> numInputsBuilder, stat_SecParam, &commBufferLen);

		sendBoth(writeSocket, commBuffer, commBufferLen);
	}
	else
	{
		commBuffer = (unsigned char *) calloc(1, sizeof(unsigned char));
		sendBoth(writeSocket, commBuffer, 1);
	}

	*J_setSize = count;

	return J_Set;
}


unsigned char *serialiseBuilderInputBits_L_2013_HKE(struct Circuit **circuitsArray, unsigned char *inputArray, int inputLength,
													unsigned char *J_set, int numCircuits, int *outputLength)
{
	struct wire *tempWire;
	unsigned char *outputBuffer;
	int i, j, bufferIndex = 0;


	(*outputLength) = 0;
	for(i = 0; i < numCircuits; i ++)
	{
		if(0x00 == J_set[i])
		{
			(*outputLength) += inputLength;
		}
	}

	outputBuffer = (unsigned char *) calloc((*outputLength), sizeof(unsigned char));
	for(i = 0; i < numCircuits; i ++)
	{
		if(0x00 == J_set[i])
		{
			for(j = 0; j < inputLength; j ++)
			{
				tempWire = circuitsArray[i] -> gates[j] -> outputWire;
				outputBuffer[bufferIndex] = inputArray[j] ^ (tempWire -> wirePerm & 0x01);

				bufferIndex ++;
			}
		}
	}

	return outputBuffer;
}
