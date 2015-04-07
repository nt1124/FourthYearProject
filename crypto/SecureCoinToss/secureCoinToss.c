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


	sendBoth(writeSocket, ownJ_Set, J_setLength);
	free(commBuffer);

	partnerJ_Set = receiveBoth(readSocket, commBufferLen);

	for(i = 0; i < J_setLength; j ++)
	{
		sumOfJs += (int) partnerJ_Set[i];
	}

	if(sumOfJs != numCircuits / 2)
	{
		free(partnerJ_Set);
		return NULL
	}


	return partnerJ_Set;
}