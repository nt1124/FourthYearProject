struct eccPoint ***computeNaorPinkasInputsForJSet(struct eccPoint *C, mpz_t **aLists, int numInputs, int numCircuits,
												struct eccParams *params, unsigned char *J_set)
{
	struct eccPoint ***output = (struct eccPoint ***) calloc(numCircuits, sizeof(struct eccPoint **));
	struct eccPoint **preComputes = preComputePoints(params -> g, 512, params), *invG_a1, *G_a1;
	int i, j, index;


	for(i = 0; i < numCircuits; i ++)
	{

		if(0x01 == J_set[i])
		{
			output[i] = (struct eccPoint **) calloc(2 * numInputs, sizeof(struct eccPoint *));
			index = 0;

			for(j = 0; j < numInputs; j ++)
			{
				output[i][index] = windowedScalarFixedPoint(aLists[i][index], params -> g, preComputes, 9, params);

				G_a1 = windowedScalarFixedPoint(aLists[i][index + 1], params -> g, preComputes, 9, params);
				invG_a1 = invertPoint(G_a1, params);
				// invG_a1 = windowedScalarFixedPoint(aLists[i][index + 1], params -> g, preComputes, 9, params);

				output[i][index + 1] = groupOp(C, invG_a1, params);
				clearECC_Point(invG_a1);

				index += 2;
			}
		}
	}

	return output;
}


// Function that gets the OT inputs as an array from the circuits.
unsigned char ***getAllInputKeysSymm(struct Circuit **circuitsArray, int numCircuits, int partyID)
{
	unsigned char ***allKeys = (unsigned char ***) calloc(2, sizeof(unsigned char **));
	struct wire *tempWire;
	int i, j, k = 0, numKeysToStore, gateIndex;
	int numInputsBuilder, numInputsExecutor;


	numInputsBuilder = circuitsArray[0] -> numInputsBuilder;
	numInputsExecutor = circuitsArray[0] -> numInputsExecutor;
	numKeysToStore = circuitsArray[0] -> numInputsExecutor * numCircuits;


	allKeys[0] = (unsigned char **) calloc(numKeysToStore, sizeof(unsigned char *));
	allKeys[1] = (unsigned char **) calloc(numKeysToStore, sizeof(unsigned char *));

	for(i = 0; i < numInputsExecutor; i ++)
	{
		gateIndex = i + numInputsBuilder * partyID;

		for(j = 0; j < numCircuits; j ++)
		{
			tempWire = circuitsArray[j] -> gates[gateIndex] -> outputWire;
			allKeys[0][k] = tempWire -> outputGarbleKeys -> key0;
			allKeys[1][k] = tempWire -> outputGarbleKeys -> key1;

			k ++;
		}
	}

	return allKeys;
}


void setInputsFromNaorPinkas(struct Circuit **circuitsArray_Own, unsigned char **output, int numCircuits, int partyID)
{
	int i, j, iOffset, gateIndex, numInputsBuilder, numInputsExecutor;
	struct wire *tempWire;
	unsigned char value;


	numInputsBuilder = circuitsArray_Own[0] -> numInputsBuilder;
	numInputsExecutor = circuitsArray_Own[0] -> numInputsExecutor;


	for(i = 0; i < numInputsExecutor; i ++)
	{
		iOffset = numCircuits * i;
		gateIndex = i + numInputsBuilder * (1 - partyID);

		for(j = 0; j < numCircuits; j ++)
		{
			tempWire = circuitsArray_Own[j] -> gates[gateIndex] -> outputWire;
			tempWire -> wireOutputKey = (unsigned char *) calloc(16, sizeof(unsigned char));
			memcpy(tempWire -> wireOutputKey, output[iOffset + j], 16);
		}
	}

}



unsigned char *jSetRevealSerialise(struct eccPoint ***NaorPinkasInputs, mpz_t **aList, unsigned char *inputBitsOwn,
								struct HKE_Output_Struct_Builder *outputStruct, ub4 **circuitSeeds,
								unsigned char *jSet, int numInputs, int numOutputs, int numCircuits, int *outputLength)
{
	unsigned char *outputBuffer;
	int i, j, k, tempOffset = 0;
	int aListLen = 0, sharesLen = 0, seedLengths = (numCircuits / 2) * 256 * sizeof(ub4);

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

			for(i = 0; i < numOutputs; i ++)
			{
				sharesLen += sizeOfSerialMPZ(outputStruct -> scheme0Array[i] -> shares[j]);
				sharesLen += sizeOfSerialMPZ(outputStruct -> scheme1Array[i] -> shares[j]);
			}
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

	(*outputLength) += (aListLen + sharesLen + seedLengths);
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

			for(i = 0; i < numOutputs; i ++)
			{
				serialiseMPZ(outputStruct -> scheme0Array[i] -> shares[j], outputBuffer, &tempOffset);
				serialiseMPZ(outputStruct -> scheme1Array[i] -> shares[j], outputBuffer, &tempOffset);
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


struct jSetRevealHKE *jSetRevealDeserialise(unsigned char *inputBuffer, unsigned char *jSet, int numInputs, int numOutputs, int numCircuits)
{
	struct jSetRevealHKE *output = (struct jSetRevealHKE *) calloc(1, sizeof(struct jSetRevealHKE));
	mpz_t *tempMPZ;
	int i, j, k, tempOffset = 0;


	output -> aListRevealed = (mpz_t **) calloc(numCircuits, sizeof(mpz_t *));
	output -> outputWireShares =  (mpz_t **) calloc(numOutputs, sizeof(mpz_t *));
	output -> revealedSeeds = (ub4 **) calloc(numCircuits, sizeof(ub4*));
	output -> builderInputsEval = (struct eccPoint ***) calloc(numCircuits, sizeof(struct eccPoint **));


	for(i = 0; i < numCircuits; i ++)
	{
		output -> outputWireShares[i] = (mpz_t *) calloc(2 * numOutputs, sizeof(mpz_t));
	}

	for(j = 0; j < numCircuits; j ++)
	{
		if(0x01 == jSet[j])
		{
			k = 0;
			output -> aListRevealed[j] = (mpz_t *) calloc(2 * numInputs, sizeof(mpz_t));
			output -> revealedSeeds[j] = (ub4 *) calloc(256, sizeof(ub4));

			for(i = 0; i < numInputs; i ++)
			{
				tempMPZ = deserialiseMPZ(inputBuffer, &tempOffset);
				mpz_init_set(output -> aListRevealed[j][k], *tempMPZ);
				k ++;

				tempMPZ = deserialiseMPZ(inputBuffer, &tempOffset);
				mpz_init_set(output -> aListRevealed[j][k], *tempMPZ);
				k ++;
			}

			k = 0;
			for(i = 0; i < numOutputs; i ++)
			{
				tempMPZ = deserialiseMPZ(inputBuffer, &tempOffset);
				mpz_init_set(output -> outputWireShares[j][k], *tempMPZ);
				k ++;

				tempMPZ = deserialiseMPZ(inputBuffer, &tempOffset);
				mpz_init_set(output -> outputWireShares[j][k], *tempMPZ);
				k ++;
			}

			memcpy(output -> revealedSeeds[j], inputBuffer + tempOffset, 256 * sizeof(ub4));
			tempOffset += 256 * sizeof(ub4);
		}
	}

	for(j = 0; j < numCircuits; j ++)
	{
		if(0x00 == jSet[j])
		{
			output -> builderInputsEval[j] = (struct eccPoint **) calloc(numInputs, sizeof(struct eccPoint *));
			for(i = 0; i < numInputs; i ++)
			{
				output -> builderInputsEval[j][i] = deserialise_ECC_Point(inputBuffer, &tempOffset);
			}
		}
	}


	return output;
}


struct HKE_Output_Struct_Builder *exchangePublicBoxesOfSchemes(int writeSocket, int readSocket, struct HKE_Output_Struct_Builder *ownInput, int numOutputs, int numCircuits)
{
	struct HKE_Output_Struct_Builder *output = (struct HKE_Output_Struct_Builder *) calloc(1, sizeof(struct HKE_Output_Struct_Builder));
	int t, commBufferLen = 0, bufferOffset = 0;
	unsigned char *commBuffer;


	t = (numCircuits / 2);


	commBuffer = serialisePubBoxes(ownInput -> scheme0Array, numOutputs, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);

	commBuffer = serialisePubBoxes(ownInput -> scheme1Array, numOutputs, &commBufferLen);
	sendBoth(writeSocket, commBuffer, commBufferLen);
	free(commBuffer);


	commBufferLen = 0;
	bufferOffset = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	output -> scheme0Array = deserialisePubBoxes(commBuffer, t, numCircuits, numOutputs, &bufferOffset);
	free(commBuffer);

	commBufferLen = 0;
	bufferOffset = 0;
	commBuffer = receiveBoth(readSocket, commBufferLen);
	output -> scheme1Array = deserialisePubBoxes(commBuffer, t, numCircuits, numOutputs, &bufferOffset);
	free(commBuffer);


	return output;
}


int verifyRevealedOutputs(struct HKE_Output_Struct_Builder *outputStruct_Partner, struct jSetRevealHKE *output, unsigned char *jSetPartner,
						int numCircuits, int numOutputs, struct DDH_Group *group)
{
	int i, j, k, verified = 0;


	for(i = 0; i < numCircuits; i ++)
	{
		if(0x01 == jSetPartner[i])
		{
			k = 0;
			for(j = 0; j < numOutputs; j ++)
			{
				verified |= VSS_Verify(outputStruct_Partner -> scheme0Array[j] -> pub, output -> outputWireShares[i][k], i + 1, group);
				k ++;

				verified |= VSS_Verify(outputStruct_Partner -> scheme1Array[j] -> pub, output -> outputWireShares[i][k], i + 1, group);
				k ++;
			}
		}
	}


	return verified;
}


unsigned char *Step5_CalculateLogarithms(struct eccPoint ***NaorPinkasInputs, mpz_t **aLists, struct OT_NP_Receiver_Query **queries_Own,
										struct eccParams *params, unsigned char *inputBitsOwn, unsigned char *J_SetPartner,
										int numCircuits, int numInputs, int *outputLength)
{
	unsigned char *outputBuffer;
	mpz_t **logsMPZ = (mpz_t **) calloc(numCircuits, sizeof(mpz_t *)), tempMPZ;
	int i, j, k, bufferOffset = 0;


	*outputLength = 0;
	mpz_init(tempMPZ);

	for(i = 0; i < numCircuits; i ++)
	{
		if(0x00 == J_SetPartner[i])
		{
			logsMPZ[i] = (mpz_t *) calloc(numInputs, sizeof(mpz_t));
			for(j = 0; j < numInputs; j ++)
			{
				k = 2 * j + inputBitsOwn[j];
				mpz_init(logsMPZ[i][j]);

				if(0x00 == inputBitsOwn[j])
				{
					mpz_sub(tempMPZ, aLists[i][k], queries_Own[j] -> k);
				}
				else
				{
					mpz_sub(tempMPZ, queries_Own[j] -> k, aLists[i][k]);
				}
				mpz_mod(logsMPZ[i][j], tempMPZ, params -> n);

				(*outputLength) += sizeOfSerialMPZ(logsMPZ[i][j]);
			}
		}
	}
	mpz_clear(tempMPZ);

	outputBuffer = (unsigned char *) calloc(*outputLength, sizeof(unsigned char));

	// Serialise the list of logarithms for sending.
	for(i = 0; i < numCircuits; i ++)
	{
		if(0x00 == J_SetPartner[i])
		{
			for(j = 0; j < numInputs; j ++)
			{
				//
				serialiseMPZ(logsMPZ[i][j], outputBuffer, &bufferOffset);
				mpz_clear(logsMPZ[i][j]);
			}
			free(logsMPZ[i]);
		}
	}
	free(logsMPZ);


	return outputBuffer;
}


int Step5_CheckLogarithms(unsigned char *inputBuffer, struct eccPoint ***builderInputsEval, struct eccPoint **queries_Partner,
						struct eccParams *params, unsigned char *J_setOwn, int numCircuits, int numInputs, int *bufferOffset)
{
	struct eccPoint *invH, *vOverH, *gPow;
	mpz_t *tempMPZ;
	int i, j, k, logarithmsChecked = 0, tempOffset = *bufferOffset;


	for(i = 0; i < numCircuits; i ++)
	{
		if(0x00 == J_setOwn[i])
		{
			for(j = 0; j < numInputs; j ++)
			{
				tempMPZ = deserialiseMPZ(inputBuffer, &tempOffset);

				invH = invertPoint(queries_Partner[j], params);
				vOverH = groupOp(builderInputsEval[i][j], invH, params);
				gPow = windowedScalarPoint(*tempMPZ, params -> g, params);

				logarithmsChecked |= eccPointsEqual(vOverH, gPow);
			}
		}
	}


	return logarithmsChecked;
}
