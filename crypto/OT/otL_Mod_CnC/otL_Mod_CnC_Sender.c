


// Really this is just deserialising a params_CnC from bufferReceived
struct params_CnC_ECC *setup_CnC_OT_Mod_Sender(unsigned char *bufferReceived)
{
	struct params_CnC_ECC *params = (struct params_CnC_ECC*) calloc(1, sizeof(struct params_CnC_ECC));
	int i, bufferOffset = 0;

	params -> params = deserialiseECC_Params(bufferReceived, &bufferOffset);
	params -> crs = deserialise_CRS_ECC(bufferReceived, &bufferOffset);

	return params;
}


struct params_CnC_ECC *setup_CnC_OT_Mod_Full_Sender(int writeSocket, int readSocket, gmp_randstate_t state)
{
	struct params_CnC_ECC *params_S;
	unsigned char *commBuffer;
	int bufferLength = 0;

	commBuffer = receiveBoth(readSocket, bufferLength);
	params_S = setup_CnC_OT_Mod_Sender(commBuffer);
	free(commBuffer);

	ZKPoK_DL_Verifier(writeSocket, readSocket, params_S -> params,
					params_S -> params -> g, params_S -> crs -> g_1, state);

	return params_S;
}



