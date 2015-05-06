#ifndef CIRCUIT_CHECKER_UTILS
#define CIRCUIT_CHECKER_UTILS


int compareGate(struct gate *gateA, struct gate *gateB)
{
	int output = 0, i, j;


	output |= (0x01 ^ (gateA -> numInputs == gateB -> numInputs));
	output |= (0x01 ^ (gateA -> outputTableSize == gateB -> outputTableSize));

	for(i = 0; i < gateA -> numInputs; i ++)
	{
		output |= (0x01 ^ (gateA -> inputIDs[i] == gateB -> inputIDs[i]));
	}

	for(i = 0; i < gateA -> outputTableSize; i ++)
	{
		output |= memcmp(gateA -> encOutputTable[i], gateB -> encOutputTable[i], 16);
	}
		

	return output;
}

int compareGateOrWirePair(struct gateOrWire *gateA, struct gateOrWire *gateB)
{
	int output = 0, i;

	output |= (0x01 ^ (gateA -> G_ID == gateB -> G_ID));


	if(NULL != gateA -> gatePayload && NULL != gateB -> gatePayload)
	{
		output |= compareGate(gateA -> gatePayload, gateB -> gatePayload);
	}
	else
	{
		output |= (0x01 ^ (gateA -> gatePayload == gateB -> gatePayload));

		output |= memcmp(gateA -> outputWire -> outputGarbleKeys -> key0, gateB -> outputWire -> outputGarbleKeys -> key0, 16);
		output |= memcmp(gateA -> outputWire -> outputGarbleKeys -> key1, gateB -> outputWire -> outputGarbleKeys -> key1, 16);
	}

	return output;
}



// Are two circuits the same?
int compareCircuit(struct RawCircuit *baseCircuit, struct Circuit *circuitA, struct Circuit *circuitB)
{
	int temp = 0, temp2;
	int i;


	// Note we don't need to compare the input wires, if any of the input wires are
	// wrong the gates using the input wires will be wrong
	for(i = baseCircuit -> numInputs; i < baseCircuit -> numGates; i ++)
	{
		temp2 = compareGateOrWirePair(circuitA -> gates[i], circuitB -> gates[i]);
		temp |= temp2;
	}


	return temp;
}



// Function that was used for debugging the circuit comparison code when it was a wee sub-routine.
void testCircuitComp(char *circuitFilepath)
{
	struct RawCircuit *rawInputCircuit = readInCircuit_Raw(circuitFilepath);
	struct Circuit *garbledCircuit1, *garbledCircuit2;

	int i, temp = 0, tempTemp;

	gmp_randstate_t *state;
	struct public_builderPRS_Keys *public_inputs;
	struct secret_builderPRS_Keys *secret_inputs;
	struct eccPoint ***consistentInputs;
	struct eccParams *params;

	randctx ctx1, ctx2;


	getIsaacContext(&ctx1);
	getIsaacContext(&ctx2);
	state = seedRandGen();

	params = initBrainpool_256_Curve();
	secret_inputs = generateSecrets(rawInputCircuit -> numInputs_P1, 1, params, *state);
	public_inputs = computePublicInputs(secret_inputs, params);


	consistentInputs = getAllConsistentInputsAsPoints(secret_inputs -> secret_circuitKeys, public_inputs -> public_keyPairs, params, 1, rawInputCircuit -> numInputs_P1);

	garbledCircuit1 = readInCircuit_FromRaw_Seeded_ConsistentInput(&ctx1, rawInputCircuit, consistentInputs[0], 0, params);
	garbledCircuit2 = readInCircuit_FromRaw_Seeded_ConsistentInput(&ctx2, rawInputCircuit, consistentInputs[0], 0, params);


	temp = compareCircuit(rawInputCircuit, garbledCircuit1, garbledCircuit2);
	printf("Comparison yield... %d\n", temp);
}




#endif