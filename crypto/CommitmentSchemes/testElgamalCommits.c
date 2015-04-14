// #include "../cryptoUtil.h"
// #include "../../circuits/timingUtils.c"

int elgamal_commit_main()
{
	const int numberOfTests = 200;
	srand( time(NULL) );
	clock_t c_0, c_1;

	gmp_randstate_t *state = seedRandGen();
	int i, bufferOffset, bufferSize1, outputLength;

	struct commit_batch_params *params;
	unsigned char **trueCommit = (unsigned char **) calloc(numberOfTests, sizeof(unsigned char *));
	unsigned char *fakeCommit;
	unsigned char *buffer, *result;


	struct elgamal_commit_box **listOfBoxes_C = (struct elgamal_commit_box **) calloc(numberOfTests, sizeof(struct elgamal_commit_box *));
	struct elgamal_commit_box **listOfBoxes_R = (struct elgamal_commit_box **) calloc(numberOfTests, sizeof(struct elgamal_commit_box *));
	struct elgamal_commit_key **listOfKeys_C = (struct elgamal_commit_key **) calloc(numberOfTests, sizeof(struct elgamal_commit_key *));
	struct elgamal_commit_key **listOfKeys_R;

	mpz_t *fakeX = (mpz_t*) calloc(1, sizeof(mpz_t));
	mpz_init(*fakeX);


	for(i = 0; i < numberOfTests; i ++)
	{
		trueCommit[i] = generateRandBytes(16, 16);
	}
	fakeCommit = generateRandBytes(16, 16);
	convertBytesToMPZ(fakeX, fakeCommit, 16);


	c_0 = clock();

	printf("Here\n");
	params = generate_commit_params(1024, *state);

	c_1 = clock();
	printf("\nCPU time to generate group  : %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);

	c_0 = clock();


	bufferOffset = 0;
	for(i = 0; i < numberOfTests; i ++)
	{
		listOfBoxes_C[i] = init_commit_box();
		listOfKeys_C[i] = init_commit_key();
		create_commit_box_key(params, trueCommit[i], 16, *state, listOfBoxes_C[i], listOfKeys_C[i]);
		
		//listOfKeys[i] = single_commit_elgamal_C(params, trueCommit[i], 16, buffer, &bufferOffset, *state);
	}

	bufferSize1 = 0;
	buffer = serialiseAllC_Boxes_1D(listOfBoxes_C, numberOfTests, &bufferSize1);

	bufferOffset = 0;
	for(i = 0; i < numberOfTests; i ++)
	{
		listOfBoxes_R[i] = single_commit_elgamal_R(buffer, &bufferOffset);
	}
	

	c_1 = clock();
	printf("\nCPU time for n many commits  : %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);

	printf("Committed\n");

	c_0 = clock();

	bufferSize1 = 0;
	bufferOffset = 0;
	buffer = serialiseAllK_Boxes_1D(listOfKeys_C, 16, numberOfTests, &bufferSize1);
	listOfKeys_R = deserialiseAllK_Boxes_1D(buffer, 16, numberOfTests, &bufferOffset);
	

	bufferOffset = 0;
	for(i = 0; i < numberOfTests; i ++)
	{
		result = single_decommit_elgamal_R(params, listOfBoxes_R[i], listOfKeys_R[i], 16, &outputLength);
		if( NULL == result)
		{
			printf("NULL results          Failed on index %02d\n", i);
		}
		else if(0 != memcmp(result, trueCommit[i], 16))
		{
			printf("result != trueCommit  Failed on index %02d\n", i);
		}
		else if(0 == i % (numberOfTests / 20))
		{
			printf("Successful test of index %02d\n", i);
		}
	}

	bufferOffset = 0;
	for(i = 0; i < numberOfTests; i ++)
	{
		result = single_decommit_elgamal_R(params, listOfBoxes_R[i], listOfKeys_R[i], 16, &outputLength);
		if(NULL != result && 0 != memcmp(result, trueCommit[i], 16))
		{
			printf("Failed on index %02d\n", i);
		}
		else if(0 == i % (numberOfTests / 20))
		{
			printf("Successful test of index %02d\n", i);
		}
	}


	c_1 = clock();

	printf("\nCPU time for 2n many decommits  : %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);

	for(i = 0; i < numberOfTests; i ++)
	{
		free(listOfBoxes_C[i]);
		free(listOfBoxes_R[i]);
		free(listOfKeys_C[i]);
		free(listOfKeys_R[i]);
		free(trueCommit[i]);
	}
	free(trueCommit);
	free(fakeX);
	free(fakeCommit);
}
