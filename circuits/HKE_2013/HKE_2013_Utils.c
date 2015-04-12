struct eccPoint ***computeNaorPinkasInputsForJSet(struct eccPoint *C, mpz_t **aLists, int numInputs, int numCircuits,
												struct eccParams *params, unsigned char *J_set)
{
	struct eccPoint ***output = (struct eccPoint ***) calloc(numCircuits, sizeof(struct eccPoint **));
	struct eccPoint **preComputes = preComputePoints(params -> g, 512, params), *invG_a1;
	int i, j, index;


	for(i = 0; i < numCircuits; i ++)
	{

		if(0x01 == J_set[i])
		{
			printf("Checkpoint 2.%d  -  %d\n", i, numInputs);
			fflush(stdout);
			output[i] = (struct eccPoint **) calloc(2 * numInputs, sizeof(struct eccPoint *));
			index = 0;

			for(j = 0; j < numInputs; j ++)
			{
				printf("Checkpoint 2.%d.%d  A\n", i, j);
				fflush(stdout);
				printf("Checkpoint 2.%d.%d  B\n", i, j);
				fflush(stdout);
				output[i][index] = windowedScalarFixedPoint(aLists[i][index], params -> g, preComputes, 9, params);

				printf("Checkpoint 2.%d.%d  C\n", i, j);
				fflush(stdout);
				invG_a1 = windowedScalarFixedPoint(aLists[i][index + 1], params -> g, preComputes, 9, params);

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



unsigned char *jSetRevealSerialise(mpz_t **aList, struct HKE_Output_Struct_Builder *outputStruct, ub4 **circuitSeeds,
									unsigned char *jSet, int numInputs, int numOutputs, int numCircuits, int *outputLength)
{
	unsigned char *outputBuffer;
	int i, j, k, tempOffset = 0;
	int aListLen = 0, sharesLen = 0, seedLengths = (numCircuits / 2) * 256 * sizeof(ub4);


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
	}

	*outputLength = aListLen + sharesLen + seedLengths;
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

	return outputBuffer;
}


struct jSetRevealHKE *jSetRevealDeserialise(unsigned char *inputBuffer, unsigned char *jSet, int numInputs, int numOutputs, int numCircuits)
{
	struct jSetRevealHKE *output = (struct jSetRevealHKE *) calloc(1, sizeof(struct jSetRevealHKE));
	mpz_t *tempMPZ;
	int i, j, k, tempOffset = 0;


	output -> aListRevealed = (mpz_t **) calloc(numCircuits, sizeof(mpz_t *));
	output -> ouputWireShares =  (mpz_t **) calloc(numOutputs, sizeof(mpz_t *));
	output -> revealedSeeds = (ub4 **) calloc(numCircuits, sizeof(ub4*));


	// for(i = 0; i < numOutputs; i ++)
	// {
	// 	output -> ouputWireShares[i] = (mpz_t *) calloc(2 * numCircuits, sizeof(mpz_t));
	// }
	for(i = 0; i < numCircuits; i ++)
	{
		output -> ouputWireShares[i] = (mpz_t *) calloc(2 * numOutputs, sizeof(mpz_t));
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
				mpz_init_set(output -> ouputWireShares[j][k], *tempMPZ);
				k ++;

				tempMPZ = deserialiseMPZ(inputBuffer, &tempOffset);
				mpz_init_set(output -> ouputWireShares[j][k], *tempMPZ);
				k ++;
			}

			memcpy(output -> revealedSeeds[j], inputBuffer + tempOffset, 256 * sizeof(ub4));
			tempOffset += 256 * sizeof(ub4);
		}
	}

	return output;
}


void testTime(mpz_t **aList, struct jSetRevealHKE *revealedDeserialised, unsigned char *jSetOwn, unsigned char *jSetPartner, int numInputs, int numOutputs, int numCircuits)
{
	int i, j, k;


	for(j = 0; j < numCircuits; j ++)
	{
		printf(">>>>>>>>>   %d\n", j);
		if(0x01 == jSetPartner[j])
		{
			k = 0;
			for(i = 0; i < numInputs; i ++)
			{
				gmp_printf("%Zd\n", aList[j][k]);
				k ++;

				gmp_printf("%Zd\n\n", aList[j][k]);
				k ++;
			}
		}
	}


	for(j = 0; j < numCircuits; j ++)
	{
		printf(">>>>>>>>>   %d\n", j);
		if(0x01 == jSetPartner[j])
		{
			k = 0;
			for(i = 0; i < numInputs; i ++)
			{
				gmp_printf("%Zd\n", revealedDeserialised -> aListRevealed[j][k]);
				k ++;

				gmp_printf("%Zd\n\n", revealedDeserialised -> aListRevealed[j][k]);
				k ++;
				fflush(stdout);
			}
		}
	}
}