#include "../cryptoUtil.h"

int main(int argc, char *argv[])
{
	const int numberOfTests = 500;
    srand( time(NULL) );

	gmp_randstate_t *state = seedRandGen();
	int securityParam = 1024, i, bufferOffset, bufferSize1;
	struct commit_batch_params *params;

	unsigned char **trueCommit = (unsigned char **) calloc(numberOfTests, sizeof(unsigned char *));
	unsigned char *fakeCommit;
	unsigned char *buffer, result;

	struct elgamal_commit_box **listOfBoxes = (struct elgamal_commit_box **) calloc(numberOfTests, sizeof(struct elgamal_commit_box *));
	struct elgamal_commit_key **listOfKeys = (struct elgamal_commit_key **) calloc(numberOfTests, sizeof(struct elgamal_commit_key *));

	mpz_t *fakeX = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_init(*fakeX);

	for(i = 0; i < numberOfTests; i ++)
	{
		trueCommit[i] = generateRandBytes(16, 16);
	}
	fakeCommit = generateRandBytes(16, 16);
	convertBytesToMPZ(fakeX, fakeCommit, 16);


	printf("Generated random test data\n");
	params = generate_commit_params(securityParam, *state);
	printf("Generated commitment parameters\n");

	bufferSize1 = getSerialSizeElgamal(params -> group, numberOfTests, 2);
	buffer = (unsigned char *) calloc(bufferSize1, sizeof(unsigned char));

	bufferOffset = 0;
	for(i = 0; i < numberOfTests; i ++)
	{
		listOfKeys[i] = single_commit_elgamal_C(params, trueCommit[i], 16, buffer, &bufferOffset, *state);
	}

	bufferOffset = 0;
	for(i = 0; i < numberOfTests; i ++)
	{
		listOfBoxes[i] = single_commit_elgamal_R(params, buffer, &bufferOffset);
	}

	printf("Committed\n");

	bufferOffset = 0;
	for(i = 0; i < numberOfTests; i ++)
	{
		single_decommit_elgamal_C(params, listOfKeys[i], buffer, &bufferOffset);
	}

	bufferOffset = 0;
	for(i = 0; i < numberOfTests; i ++)
	{
		result = single_decommit_elgamal_R(params, listOfBoxes[i], buffer, &bufferOffset);
		if(0 == result)
		{
			printf("Failed on index %02d\n", i);
		}
		/*
		else if(0 == i % (numberOfTests / 20))
		{
			printf("Successful test of index %02d\n", i);
		}
		*/
	}


	bufferOffset = 0;
	for(i = 0; i < numberOfTests; i ++)
	{
		mpz_set(listOfKeys[i] -> x, *fakeX);
		single_decommit_elgamal_C(params, listOfKeys[i], buffer, &bufferOffset);
	}

	bufferOffset = 0;
	for(i = 0; i < numberOfTests; i ++)
	{
		result = single_decommit_elgamal_R(params, listOfBoxes[i], buffer, &bufferOffset);
		if(1 == result)
		{
			printf("Failed on index %02d\n", i);
		}
		/*
		else if(0 == i % (numberOfTests / 20))
		{
			printf("Successful test of index %02d\n", i);
		}
		*/
	}



	for(i = 0; i < numberOfTests; i ++)
	{
		free(listOfBoxes[i]);
		free(listOfKeys[i]);
		free(trueCommit[i]);
	}
	free(trueCommit);
	free(fakeX);
	free(fakeCommit);
}