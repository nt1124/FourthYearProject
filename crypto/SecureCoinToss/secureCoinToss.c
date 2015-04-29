/*	Basically...

	1) Both parties randomly generate their bit strings
	2) Both parties commit to their bit strings to the other.
	3) Both parties check to make sure their inputs are not the same.
		If this is the case redo.
	4) 


	BUT, for now we will just SEND a J-set.
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



randctx *getRandCtxByCoinToss(int writeSocket, int readSocket, randctx *ownCTX, gmp_randstate_t *state, int partyID)
{
	struct commit_batch_params *ownParams, *partnerParams;
	struct elgamal_commit_box *ownC_Box, *partnerC_Box;
	struct elgamal_commit_key *ownK_Box, *partnerK_Box;

	ub4 *XOR_seed = (ub4 *) calloc(256, sizeof(ub4));
	randctx *ctx = (randctx *) calloc(1, sizeof(randctx *));
	unsigned char *ownSeedBytes, *partnerSeedBytes, *commBufferWrite, *commBufferRead;
	int i, commBufferWriteLen, commBufferReadLen;


	ownSeedBytes = generateIsaacRandBytes(ownCTX, 256 *sizeof(ub4), 256 * sizeof(ub4));

	ownParams = generate_commit_params(1024, *state);
	commBufferWrite = serialise_commit_batch_params(ownParams, &commBufferWriteLen);
	if(0 == partyID)
	{
		sendBoth(writeSocket, commBufferWrite, commBufferWriteLen);
	}

	ownC_Box = init_commit_box();
	ownK_Box = init_commit_key();

	setIsaacContextFromSeed(ctx, XOR_seed);

	return ctx;
}




