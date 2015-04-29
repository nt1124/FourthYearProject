/*	Basically...

	1) Both parties randomly generate their bit strings
	2) Both parties commit to their bit strings to the other.
	3) Both parties check to make sure their inputs are not the same.
		If this is the case redo.
	4) Both parties decommit to their bit strings.
	5) Parties XOR the inputs and use the result to Seed a random number generator.
*/



unsigned char *getPartnerJ_Set(int writeSocket, int readSocket, unsigned char *ownJ_Set, int jSetSize, int numCircuits)
{
	unsigned char *partnerJ_Set;
	int commBufferLen = 0, i, sumOfJs = 0;


	sendBoth(writeSocket, ownJ_Set, numCircuits);

	partnerJ_Set = receiveBoth(readSocket, commBufferLen);

	for(i = 0; i < numCircuits; i ++)
	{
		sumOfJs += (int) partnerJ_Set[i];
	}

	if(sumOfJs != numCircuits / 2)
	{
		free(partnerJ_Set);
		return NULL;
	}


	return partnerJ_Set;
}




// Note because 256 * sizeof(ub4) is much bigger than 16 we have definitely not got Perfectly binding here.
randctx *getRandCtxByCoinToss(int writeSocket, int readSocket, randctx *ownCTX, gmp_randstate_t *state, int partyID)
{
	struct commit_batch_params *ownParams, *partnerParams;

	struct elgamal_commit_box *ownC_Box, *partnerC_Box;
	struct elgamal_commit_key *ownK_Box, *partnerK_Box;

	ub4 *XOR_seed = (ub4 *) calloc(256, sizeof(ub4)), tempUB4;
	randctx *ctx;
	unsigned char *ownSeedBytes, *partnerSeedBytes, *commBufferWrite, *commBufferRead;
	int i, commBufferWriteLen, commBufferReadLen, bufferOffset;
	int commitLen = 256 * sizeof(ub4);


	ownSeedBytes = generateIsaacRandBytes(ownCTX, commitLen, commitLen);



	ownParams = generate_commit_params(1024, *state);
	commBufferWrite = serialise_commit_batch_params(ownParams, &commBufferWriteLen);

	commBufferRead = sendReceiveExchange(writeSocket, readSocket, commBufferWrite,
										commBufferWriteLen, &commBufferReadLen, partyID);
	bufferOffset = 0;
	partnerParams = deserialise_commit_batch_params(commBufferRead, &bufferOffset);
	free(commBufferWrite);
	free(commBufferRead);


	// Create own C-Box and K-Box.
	ownC_Box = init_commit_box();
	ownK_Box = init_commit_key();
	create_commit_box_key(ownParams, ownSeedBytes, commitLen, *state, ownC_Box, ownK_Box);


	// Exchange C-boxes.
	bufferOffset = 0;
	commBufferWriteLen = sizeOf_C_Box_Serial(ownC_Box);
	commBufferWrite = (unsigned char *) calloc(commBufferWriteLen, sizeof(unsigned char));
	serialise_elgamal_Cbox(ownC_Box, commBufferWrite, &bufferOffset);

	commBufferRead = sendReceiveExchange(writeSocket, readSocket, commBufferWrite,
										commBufferWriteLen, &commBufferReadLen, partyID);
	bufferOffset = 0;
	partnerC_Box = deserialise_elgamal_Cbox(commBufferRead, &bufferOffset);
	free(commBufferWrite);
	free(commBufferRead);


	// Exchange K-Boxes now.
	bufferOffset = 0;
	commBufferWriteLen = sizeOf_K_Box_Serial(ownK_Box, commitLen);
	commBufferWrite = (unsigned char *) calloc(commBufferWriteLen, sizeof(unsigned char));
	serialise_elgamal_Kbox(ownK_Box, commitLen, commBufferWrite, &bufferOffset);

	bufferOffset = 0;
	commBufferRead = sendReceiveExchange(writeSocket, readSocket, commBufferWrite,
										commBufferWriteLen, &commBufferReadLen, partyID);
	partnerK_Box = deserialise_elgamal_Kbox(commBufferRead, commitLen, &bufferOffset);

	// Decommit to get the partners seed
	bufferOffset = 0;
	partnerSeedBytes = single_decommit_elgamal_R(partnerParams, partnerC_Box, partnerK_Box, commitLen, &bufferOffset);
	free(commBufferWrite);
	free(commBufferRead);


	commBufferRead = XOR_TwoStrings(ownSeedBytes, partnerSeedBytes, commitLen);

	bufferOffset = 0;
	for(i = 0; i < 256; i ++)
	{
		memcpy(&tempUB4, commBufferRead + bufferOffset, sizeof(ub4));
		XOR_seed[i] = tempUB4;

		bufferOffset += sizeof(ub4);
	}

	ctx = (randctx *) calloc(1, sizeof(randctx));
	setIsaacContextFromSeed(ctx, XOR_seed);


	return ctx;
}



unsigned char **getJ_Set(randctx *ctx, int partyID, int jSetLength)
{
	unsigned char **J_Sets = (unsigned char **) calloc(2, sizeof(unsigned char *));

	J_Sets[partyID] = generateJ_Set_ISAAC(ctx, jSetLength);
	J_Sets[1 - partyID] = generateJ_Set_ISAAC(ctx, jSetLength);

	return J_Sets;
}