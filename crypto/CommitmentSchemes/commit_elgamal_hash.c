void commit_hash_elgamal_Committer()
{
	
}


void commit_hash_elgamal_Receiver()
{
	
}


struct DDH_Group *setup_hash_elgamal_Committer(int securityParam, gmp_randstate_t state, unsigned char *outputBuffer, int *bufferLength)
{
	struct DDH_Group *group = generateGroup(securityParam, state);

	

	return group;
}

	
}


void setup_hash_elgamal_Receiver(int securityParam, gmp_randstate_t state, unsigned char *inputBuffer, int bufferLength)
{
	struct DDH_Group *group = initGroupStruct();


	return group;
}