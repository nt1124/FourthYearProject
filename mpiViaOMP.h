#ifndef MPI_VIA_OMP
#define MPI_VIA_OMP

#include <omp.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>


const int numParties = 4;


int blockingMultiSend(int receiverID, long int* commChannels, long int* data, int blockSize);
long int* blockingMultiRecv(int senderID, long int* commChannels);

int blockingSingleSend(int senderID, long int* commChannels, long int data, int receiverID);
long int blockingSingleRecv(int receiverID, long int* commChannels, int senderID);

int blockingSameBroadcast(long int* commChannels, long int* data, int blockSize);
int blockingDiffBroadcast(long int* commChannels, long int** data, int blockSize);

void initRandom();
long int* getRandomData(int blockSize);

void blockingGather(long int* commChannels, long int**recvTemp);



void timeTest(long int* commChannels)
{
	clock_t begin, end;
	double time_spent;
	int id;
	int blockSize = 10000000;
	long int* sendTemp = getRandomData(blockSize);

	begin = clock();
	
	#pragma omp parallel shared(commChannels) private(id, blockSize)
	{
		id = omp_get_thread_num();

		if(0 == id)
		{
			blockingMultiSend(1, commChannels, sendTemp, blockSize);
		}
		else if(1 == id)
		{
			long int* recvTemp = blockingMultiRecv(0, commChannels);
			long int maxIndex = *(recvTemp);
		}
	}

	end = clock();

	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("Multi-send Test\n");
	printf("Elapsed: %f seconds\n", (double)time_spent);
	printf("%f seconds per 1000 long int\n", (double)(1000 * time_spent / blockSize));
}


void broadcastTestSlow(long int* commChannels)
{
	clock_t begin, end;
	double time_spent;
	int id;
	int blockSize = 10000000;
	long int* sendTemp = getRandomData(blockSize);

	begin = clock();
	
	#pragma omp parallel shared(commChannels) private(id, blockSize)
	{
		id = omp_get_thread_num();

		if(0 == id)
		{
			blockingSameBroadcast(commChannels, sendTemp, blockSize);
		}
		else
		{
			long int* recvTemp = blockingMultiRecv(0, commChannels);
			long int maxIndex = *(recvTemp);
		}
	}

	end = clock();

	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("\"Slow\" Broadcast Test\n");
	printf("Elapsed: %f seconds\n", (double)time_spent);
	printf("%f seconds per 1000 long int\n", (double)( 1000 * time_spent / (blockSize * (numParties - 1)) ));
}



void broadcastTestFast(long int* commChannels)
{
	clock_t begin, end;
	double time_spent;
	int id;
	int blockSize = 10000000;
	long int* sendTemp = getRandomData(blockSize);

	begin = clock();
	
	#pragma omp parallel shared(commChannels) private(id, blockSize)
	{
		id = omp_get_thread_num();

		if(0 == id)
		{
			fastSameBroadcast(commChannels, sendTemp, blockSize);
		}
		else
		{
			long int* recvTemp = blockingMultiRecv(0, commChannels);
			long int maxIndex = *(recvTemp);
		}
	}

	end = clock();

	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("Fast Broadcast Test\n");
	printf("Elapsed: %f seconds\n", (double)time_spent);
	printf("%f seconds per 1000 long int\n", (double)( 1000 * time_spent / (blockSize * (numParties - 1)) ));
}



void broadcastDiffTestFast(long int* commChannels)
{
	clock_t begin, end;
	double time_spent;
	int id, i;
	int blockSize = 10000000;

	long int* sendTemp[numParties - 1];
	for(i = 0; i < numParties - 1; i ++)
	{
		sendTemp[i] = getRandomData(blockSize);
	}

	begin = clock();

	#pragma omp parallel shared(commChannels) private(id)
	{
		id = omp_get_thread_num();

		if(0 == id)
		{
			fastDiffBroadcast(commChannels, sendTemp, blockSize);
		}
		else
		{
			long int* recvTemp = blockingMultiRecv(0, commChannels);
			long int maxIndex = *(recvTemp);
		}
	}

	end = clock();

	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("Fast Broadcast Diff Test\n");
	printf("Elapsed: %f seconds\n", (double)time_spent);
	printf("%f seconds per 1000 long int\n", (double)( 1000 * time_spent / (blockSize * (numParties - 1)) ));
}



void broadcastDiffTestSlow(long int* commChannels)
{
	clock_t begin, end;
	double time_spent;
	int id, i;
	int blockSize = 10000000;

	begin = clock();
	
	long int* sendTemp[numParties - 1];
	for(i = 0; i < numParties - 1; i ++)
	{
		sendTemp[i] = getRandomData(blockSize);
	}


	#pragma omp parallel shared(commChannels) private(id)
	{
		id = omp_get_thread_num();

		if(0 == id)
		{
			blockingDiffBroadcast(commChannels, sendTemp, blockSize);
		}
		else
		{
			long int* recvTemp = blockingMultiRecv(0, commChannels);
			long int maxIndex = *(recvTemp);
		}
	}

	end = clock();

	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("\"Slow\" Broadcast Diff Test\n");
	printf("Elapsed: %f seconds\n", (double)time_spent);
	printf("%f seconds per 1000 long int\n", (double)( 1000 * time_spent / (blockSize * (numParties - 1)) ));
}



void testGather(long int* commChannels)
{
	clock_t begin, end;
	double time_spent;
	int id, i, iAdjusted;
	int blockSize = 10000000;

	begin = clock();
	
	long int* sendTemp[numParties - 1];
	for(i = 0; i < numParties - 1; i ++)
	{
		sendTemp[i] = getRandomData(blockSize);
	}


	#pragma omp parallel shared(commChannels) private(id)
	{
		id = omp_get_thread_num();

		if(0 == id)
		{
			long int* recvTemp[numParties - 1];
			blockingGather(commChannels, recvTemp);
		}
		else
		{
			blockingMultiSend(0, commChannels, sendTemp[id - 1], blockSize);
		}
	}

	end = clock();

	time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("Gather Test\n");
	printf("Elapsed: %f seconds\n", (double)time_spent);
	printf("%f seconds per 1000 long int\n", (double)( 1000 * time_spent / (blockSize * (numParties - 1)) ));
}


void blockingGather(long int* commChannels, long int**recvTemp)
{
	int i, iAdjusted, ownID = omp_get_thread_num();

	for(i = 0; i < numParties; i ++)
	{
		iAdjusted = i;
		if(i != ownID)
		{
			if(i > ownID)
				iAdjusted --;

			recvTemp[iAdjusted] = blockingMultiRecv(i, commChannels);
		}
	}
}


long int getLongRandom()
{
	short x1 = rand() % USHRT_MAX;
	short x2 = rand() % USHRT_MAX;
	short x3 = rand() % USHRT_MAX;
	short x4 = rand() % SHRT_MAX;

	long int toReturn = x4;
	toReturn = (toReturn << 16) + x3;
	toReturn = (toReturn << 16) + x2;
	toReturn = (toReturn << 16) + x1;

	return toReturn; 
}


int blockingMultiSend(int receiverID, long int* commChannels, long int* data, int blockSize)
{
	int senderID = omp_get_thread_num(), i;

	if(receiverID == senderID)
		return -1;

	blockingSingleSend(receiverID, commChannels, blockSize, senderID);

	for(i = 0; i < blockSize; i ++)
	{
		blockingSingleSend(receiverID, commChannels, *(data + i), senderID);
	}

	return 1;
}


long int* blockingMultiRecv(int senderID, long int* commChannels)
{
	int receiverID = omp_get_thread_num(), i;
	long int* toReturn;
	long int blockSize;

	if(receiverID == senderID)
		return NULL;

	blockSize = blockingSingleRecv(receiverID, commChannels, senderID);
	toReturn = calloc(blockSize + 1, sizeof(long int));
	*(toReturn) = blockSize;

	for(i = 0; i < blockSize; i ++)
	{
		*(toReturn + i + 1) = blockingSingleRecv(receiverID, commChannels, senderID);
	}

	return toReturn;
}


int blockingSingleSend(int receiverID, long int* commChannels, long int data, int senderID)
{
	int index = ((receiverID * numParties) + senderID);

	while( *(commChannels + index) != -1 )	{}

	*(commChannels + index) = data;
	
	while( *(commChannels + index) != -1 )	{}
	
	return 1;
}


long int blockingSingleRecv(int receiverID, long int* commChannels, int senderID)
{
	int index = ((receiverID * numParties) + senderID);
	long int toReturn;

	while( *(commChannels + index) == -1 )	{}

	toReturn = *(commChannels + index);
	*(commChannels + index) = -1;

	return toReturn;
}


int fastSameBroadcast(long int* commChannels, long int* data, int blockSize)
{	
	int i, j, ownID = omp_get_thread_num();

	for(i = 0; i < numParties; i ++)
	{
		if(i != ownID)
		{
			blockingSingleSend(i, commChannels, blockSize, ownID);
		}
	}

	for(j = 0; j < blockSize; j ++)
	{
		for(i = 0; i < numParties; i ++)
		{	
			if(i != ownID)
			{
				blockingSingleSend(i, commChannels, *(data + j), ownID);
			}
		}
	}

	return 1;
}


int blockingSameBroadcast(long int* commChannels, long int* data, int blockSize)
{
	int i, ownID = omp_get_thread_num();

	for(i = 0; i < numParties; i ++)
	{
		if(i != ownID)
			blockingMultiSend(i, commChannels, data, blockSize);
	}

	return 1;
}


int fastDiffBroadcast(long int* commChannels, long int** data, int blockSize)
{
	int i, iAdjusted, j, ownID = omp_get_thread_num();

	for(i = 0; i < numParties; i ++)
		if(i != ownID)
			blockingSingleSend(i, commChannels, blockSize, ownID);


	for(j = 0; j < blockSize; j ++)
	{
		for(i = 0; i < numParties; i ++)
		{
			if(i != ownID)
			{
				if(i > ownID)
				{
					iAdjusted = i - 1;
				}
				blockingSingleSend(i, commChannels, data[iAdjusted][j], ownID);
			}
		}
	}

	return 1;
}


int blockingDiffBroadcast(long int* commChannels, long int** data, int blockSize)
{
	int i, iAdjusted, ownID = omp_get_thread_num();

	for(i = 0; i < numParties; i ++)
	{
		if(i != ownID)
		{
			if(i > ownID)
			{
				iAdjusted = i - 1;
			}
			blockingMultiSend(i, commChannels, *(data + iAdjusted), blockSize);
		}
	}

	return 1;
}

void initRandom()
{
	srand(time(NULL));
}


long int* getRandomData(int blockSize)
{
	long int* toReturn = calloc(blockSize, sizeof(long int));
	long int temp = 0;
	short tempShort = 0;
	int i;

	for(i = 0; i < blockSize; i ++)
	{
		*(toReturn + i) = getLongRandom();
	}

	return toReturn;
}


#endif