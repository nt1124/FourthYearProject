struct eccPoint ***ComputeAllConsistentOutputs(struct secret_builderPRS_Keys *secret_inputs, struct public_builderPRS_Keys *public_inputs,
											struct eccParams *params, int numCircuits, int numInputs)
{
	struct eccPoint ***output = (struct eccPoint ***) calloc(numCircuits, sizeof(struct eccPoint **));
	int i, j, k;


	for(i = 0; i < numCircuits; i ++)
	{
		output[i] = (struct eccPoint **) calloc(2 * numInputs, sizeof(struct eccPoint *));
		for(j = 0; j < numInputs; j ++)
		{
			k = 2 * j;
			output[i][k + 0] = windowedScalarPoint(secret_inputs -> secret_circuitKeys[i], public_inputs -> public_keyPairs[j][0], params);
			output[i][k + 1] = windowedScalarPoint(secret_inputs -> secret_circuitKeys[i], public_inputs -> public_keyPairs[j][1], params);
		}
	}


	return output;
}