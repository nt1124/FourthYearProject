// Function to take the secret sharing scheme structures. 
mpz_t *getOutputKeys(struct HKE_Output_Struct_Builder *outputStructs, int numInputs, int j)
{
	mpz_t *outputKeys = (mpz_t *) calloc(2 * numInputs, sizeof(mpz_t));
	int i, index = 0;

	for(i = 0; i < numInputs; i ++)
	{
		mpz_init_set(outputKeys[index + 0], outputStructs -> scheme0Array[i] -> shares[j]);
		mpz_init_set(outputKeys[index + 1], outputStructs -> scheme1Array[i] -> shares[j]);

		index += 2;
	}


	return outputKeys;
}


// Process a gateOrWire struct given the data.
struct gateOrWire *processGateOrWire_FromRaw_VSS_Output(randctx *ctx, struct RawGate *rawGate, struct gateOrWire **circuit,
														mpz_t key0, mpz_t key1, int numInputs1)
{
	struct gateOrWire *toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));
	unsigned char permC = 0x00, usingBuilderInput = 0xF0, *k0Bytes, *k1Bytes;
	int inputID, i, tempLength0 = 0, tempLength1 = 0;


	toReturn -> G_ID = rawGate -> G_ID;
	toReturn -> outputWire = (struct wire *) calloc(1, sizeof(struct wire));
	toReturn -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));

	toReturn -> gatePayload = processGate_FromRaw(rawGate -> numInputs, rawGate -> inputIDs, rawGate -> rawOutputTable, rawGate -> gateType);

	if('X' == rawGate -> gateType)
	{
		for(i = 0; i < toReturn -> gatePayload -> numInputs; i ++)
		{
			inputID = toReturn -> gatePayload -> inputIDs[i];
			permC ^= circuit[inputID] -> outputWire -> wirePerm;
			usingBuilderInput &= 0xE0;
		}

		toReturn -> outputWire -> wireMask ^= usingBuilderInput;
		toReturn -> outputWire -> wirePerm = permC;
	}
	else
	{
		toReturn -> outputWire -> wirePerm = getIsaacPermutation(ctx);
	}

	toReturn -> outputWire -> outputGarbleKeys = (struct bitsGarbleKeys*) calloc(1, sizeof(struct bitsGarbleKeys));
	k0Bytes = convertMPZToBytes(key0, &tempLength0);
	k1Bytes = convertMPZToBytes(key1, &tempLength1);


	toReturn -> outputWire -> outputGarbleKeys -> key0 = (unsigned char *) calloc(17, sizeof(unsigned char));
	toReturn -> outputWire -> outputGarbleKeys -> key1 = (unsigned char *) calloc(17, sizeof(unsigned char));

	memcpy(toReturn -> outputWire -> outputGarbleKeys -> key0, k0Bytes, 16);
	memcpy(toReturn -> outputWire -> outputGarbleKeys -> key1, k1Bytes, 16);
	toReturn -> outputWire -> outputGarbleKeys -> key0[16] = 0x00 ^ (0x01 & toReturn -> outputWire -> wirePerm);
	toReturn -> outputWire -> outputGarbleKeys -> key1[16] = 0x01 ^ (0x01 & toReturn -> outputWire -> wirePerm);


	encWholeOutTable(toReturn, circuit);


	free(k0Bytes);
	free(k1Bytes);


	return toReturn;
}



// Initialises an input wire, needed because input wires don't feature in the input file.
// Therefore they need to initialised separately.
struct gateOrWire *initInputWire_FromRaw_HKE_2013(randctx *ctx, int idNum, unsigned char owner, unsigned char *R,
												struct eccPoint *NP_InputsZero, struct eccPoint *NP_InputsOne, struct eccParams *params)
{
	struct gateOrWire *toReturn = (struct gateOrWire*) calloc(1, sizeof(struct gateOrWire));
	struct bitsGarbleKeys *tempOutput;

	int i;


	toReturn -> G_ID = idNum;
	toReturn -> outputWire = (struct wire *) calloc(1, sizeof(struct wire));
	toReturn -> outputWire -> wirePerm = getIsaacPermutation(ctx);
	toReturn -> outputWire -> wireOutputKey = (unsigned char*) calloc(16, sizeof(unsigned char));

	tempOutput = (struct bitsGarbleKeys*) calloc(1, sizeof(struct bitsGarbleKeys));

	tempOutput -> key0 = hashECC_Point(NP_InputsZero, 16);
	tempOutput -> key1 = hashECC_Point(NP_InputsOne, 16);


	toReturn -> outputWire -> outputGarbleKeys = tempOutput;

	toReturn -> gatePayload = NULL;
	toReturn -> outputWire -> wireMask = 0x01;
	toReturn -> outputWire -> wireOwner = owner;


	return toReturn;
}


// partyID = 1 indicates the building party inputs come first.
// partyID = 0 indicates the building party inputs come second.
struct gateOrWire **initAllInputs_FromRaw_HKE_2013(randctx *ctx, struct RawCircuit *rawInputCircuit, unsigned char *R,
												struct eccPoint **NaorPinkasInputs, int partyID, struct eccParams *params)
{
	struct gateOrWire **gates = (struct gateOrWire**) calloc(rawInputCircuit -> numGates, sizeof(struct gateOrWire*));
	int i, index = 0;
	int execPartyStartID = 0, execPartyEndID, buildingPartyStartID = 0, buildingPartyEndID;


	if(1 == partyID)
	{
		buildingPartyStartID = 0;
		execPartyStartID = rawInputCircuit -> numInputs_P1;

		buildingPartyEndID = buildingPartyStartID + rawInputCircuit -> numInputs_P1;
		execPartyEndID = execPartyStartID + rawInputCircuit -> numInputs_P2;
	}
	else if(0 == partyID)
	{
		buildingPartyStartID = rawInputCircuit -> numInputs_P1;
		execPartyStartID = 0;

		buildingPartyEndID = buildingPartyStartID + rawInputCircuit -> numInputs_P2;
		execPartyEndID = rawInputCircuit -> numInputs_P1;
	}

	// printf("%d  %d  %d  %d\n", buildingPartyStartID, buildingPartyEndID, execPartyStartID, execPartyEndID);

	for(i = buildingPartyStartID; i < buildingPartyEndID; i ++)
	{
		gates[i] = initInputWire_FromRaw_HKE_2013(ctx, i, 0xFF, R, NaorPinkasInputs[index], NaorPinkasInputs[index + 1], params);
		index += 2;
	}

	for(i = execPartyStartID; i < execPartyEndID; i ++)
	{
		gates[i] = initInputWire_FromRaw(ctx, i, 0x00, R);
		// printf("%d - %02X\n", i, gates[i] -> outputWire -> wirePerm);
	}


	return gates;
}


// Create a circuit given a file in RTL format.
struct Circuit *readInCircuit_FromRaw_HKE_2013(randctx *ctx, struct RawCircuit *rawInputCircuit, struct eccPoint **NaorPinkasInputs,
											mpz_t *outputKeysLocal, struct eccParams *params, int partyID)
{
	struct gateOrWire *tempGateOrWire;
	struct gateOrWire **gatesList;

	struct Circuit *outputCircuit = (struct Circuit*) calloc(1, sizeof(struct Circuit));
	int i, k = 0, gateIndex = 0;

	unsigned char *R = generateIsaacRandBytes(ctx, 16, 17);


	outputCircuit -> numGates = rawInputCircuit -> numGates;

	if(1 == partyID)
	{
		outputCircuit -> numInputsBuilder = rawInputCircuit -> numInputs_P1;
		outputCircuit -> numInputsExecutor = rawInputCircuit -> numInputs_P2;
		outputCircuit -> builderInputOffset = 0;
	}
	else
	{
		outputCircuit -> numInputsBuilder = rawInputCircuit -> numInputs_P2;
		outputCircuit -> numInputsExecutor = rawInputCircuit -> numInputs_P1;
		outputCircuit -> builderInputOffset = rawInputCircuit -> numInputs_P1;
	}

	// printf("%d - %d - %d\n", outputCircuit -> numInputsBuilder, outputCircuit -> numInputsExecutor, outputCircuit -> builderInputOffset);

	outputCircuit -> checkFlag = 0x00;
	outputCircuit -> numInputs = rawInputCircuit -> numInputs_P1 + rawInputCircuit -> numInputs_P2;
	outputCircuit -> numOutputs = rawInputCircuit -> numOutputs;


	// outputKeysLocal = getOutputKeys(outputStructs, rawInputCircuit -> numOutputs, j);
	gatesList = initAllInputs_FromRaw_HKE_2013( ctx, rawInputCircuit, R, NaorPinkasInputs, partyID, params );

	for(i = outputCircuit -> numInputs; i < outputCircuit -> numGates; i ++)
	{
		gateIndex = rawInputCircuit -> execOrder[i];

		if(gateIndex < outputCircuit -> numGates - outputCircuit -> numOutputs)
		{
			tempGateOrWire = processGateLine_FromRaw(ctx, rawInputCircuit -> gates[gateIndex], gatesList, R, 0, outputCircuit -> numInputs);
		}
		else
		{
			k = 2 * (gateIndex - (outputCircuit -> numGates - outputCircuit -> numOutputs));
			tempGateOrWire = processGateOrWire_FromRaw_VSS_Output(ctx, rawInputCircuit -> gates[gateIndex], gatesList,
																outputKeysLocal[k], outputKeysLocal[k + 1],
																outputCircuit -> numInputs);
		}

		if( NULL != tempGateOrWire )
		{
			*(gatesList + gateIndex) = tempGateOrWire;
		}
	}

	for(i = 0; i < outputCircuit -> numOutputs; i ++)
	{
		gateIndex = outputCircuit -> numGates - outputCircuit -> numOutputs + i;
		gatesList[gateIndex] -> outputWire -> wireMask = 0x02;
	}


	outputCircuit -> gates = gatesList;
	outputCircuit -> execOrder = rawInputCircuit -> execOrder;

	free(R);

	return outputCircuit;
}


struct eccPoint ***computeNaorPinkasInputs(struct eccPoint *C, mpz_t **aLists, int numInputs, int numCircuits, struct eccParams *params)
{
	struct eccPoint ***output = (struct eccPoint ***) calloc(numCircuits, sizeof(struct eccPoint **));
	struct eccPoint *invG_a1, *G_a1;
	int i, j, index;


	#pragma omp parallel for private(i, j, index, G_a1, invG_a1) schedule(auto)
	for(i = 0; i < numCircuits; i ++)
	{
		output[i] = (struct eccPoint **) calloc(2 * numInputs, sizeof(struct eccPoint *));
		// index = 0;
		for(j = 0; j < numInputs; j ++)
		{
			index = 2 * j;
			output[i][index] = fixedPointMultiplication(gPreComputes, aLists[i][index], params);

			G_a1 = fixedPointMultiplication(gPreComputes, aLists[i][index + 1], params);
			invG_a1 = invertPoint(G_a1, params);

			output[i][index + 1] = groupOp(C, invG_a1, params);

			clearECC_Point(invG_a1);
		}
	}

	return output;
}


mpz_t **getNaorPinkasInputs(int numInputs, int numCircuits, gmp_randstate_t state, struct eccParams *params)
{
	mpz_t **output = (mpz_t **) calloc(numCircuits, sizeof(mpz_t *));
	mpz_t *temp;
	int i, j, index;


	for(i = 0; i < numCircuits; i ++)
	{
		output[i] = (mpz_t *) calloc(2 * numInputs, sizeof(mpz_t));
		index = 0;

		for(j = 0; j < numInputs; j ++)
		{
			mpz_init(output[i][index]);
			mpz_urandomm(output[i][index], state, params -> n);

			mpz_init(output[i][index + 1]);
			mpz_urandomm(output[i][index + 1], state, params -> n);

			index += 2;
		}
	}

	return output;
}


struct HKE_Output_Struct_Builder *getOutputSecretsAndScheme(int numOutputs, int numCircuits, gmp_randstate_t state, struct DDH_Group *group)
{
	struct HKE_Output_Struct_Builder *output = (struct HKE_Output_Struct_Builder *) calloc(1, sizeof(struct HKE_Output_Struct_Builder));
	mpz_t *temp;
	int i, t;

	int j;

	t = (numCircuits / 2);

	output -> s_0Array = (mpz_t *) calloc(numOutputs, sizeof(mpz_t));
	output -> s_1Array = (mpz_t *) calloc(numOutputs, sizeof(mpz_t));

	output -> scheme0Array = (struct sharingScheme **) calloc(numOutputs, sizeof(struct sharingScheme *));
	output -> scheme1Array = (struct sharingScheme **) calloc(numOutputs, sizeof(struct sharingScheme *));

	for(i = 0; i < numOutputs; i ++)
	{
		mpz_init(output -> s_0Array[i]);
		mpz_urandomm(output -> s_0Array[i], state, group -> q);

		mpz_init(output -> s_1Array[i]);
		mpz_urandomm(output -> s_1Array[i], state, group -> q);

		output -> scheme0Array[i] = VSS_Share(output -> s_0Array[i], t, numCircuits, state, group);
		output -> scheme1Array[i] = VSS_Share(output -> s_1Array[i], t, numCircuits, state, group);
	}


	return output;
}


