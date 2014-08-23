#include "mpiViaOMP.h"

'''



'''



int main(int argc, char **argv)
{
	int np, id, indexX, indexY;
	omp_set_num_threads(numParties);

	//Grid, [ (receiver * n) + sender ]  is a message from Y to X
	long int* commChannels = (long int*) calloc(numParties * numParties, sizeof(long int));

	for(indexX = 0; indexX < numParties; indexX ++)
		for(indexY = 0; indexY < numParties; indexY ++)
			*(commChannels + ((indexY * numParties) + indexX)) = -1;


	if( 2 == argc )
	{
		if( 0 == strncmp("1", argv[1], 1) )
		{
			timeTest(commChannels);
		}
		else if(0 == strncmp("2", argv[1], 1) )
		{
			broadcastTestFast(commChannels);
		}
		else if(0 == strncmp("3", argv[1], 1) )
		{
			broadcastTestSlow(commChannels);
		}
		else if(0 == strncmp("4", argv[1], 1) )
		{
			broadcastDiffTestFast(commChannels);
		}
		else if(0 == strncmp("5", argv[1], 1) )
		{
			broadcastDiffTestSlow(commChannels);
		}
		else if(0 == strncmp("6", argv[1], 1) )
		{
			testGather(commChannels);
		}
	}
	else
	{
		printf("Error: Only one argument expected\n");
	}
	return 0;
}
