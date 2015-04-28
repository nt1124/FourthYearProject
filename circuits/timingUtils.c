#ifndef TIMING_UTILS
#define	TIMING_UTILS

#include <time.h>
#include <stdint.h>


const int OUTPUT_TIMINGS = 1;

long int raw_timespecDiff(struct timespec *timeStart, struct timespec *timeEnd)
{
	return ((timeEnd->tv_sec * 1000000000) + timeEnd->tv_nsec) -
			((timeStart->tv_sec * 1000000000) + timeStart->tv_nsec);
}


double seconds_timespecDiff(struct timespec *timeStart, struct timespec *timeEnd)
{
	double conversionFactor = 1000000;//000;
	long long int toReturn = ((timeEnd->tv_sec * 1000000) + timeEnd->tv_nsec / 1000) -
							 ((timeStart->tv_sec * 1000000) + timeStart->tv_nsec / 1000);

	double output = (double) toReturn / conversionFactor;

	return output;
}


void printTiming(struct timespec *t_0, struct timespec *t_1,
				clock_t c_0, clock_t c_1, const char *headerStr)
{
	if(1 == OUTPUT_TIMINGS)
	{
		printf("\n+++ %s\n", headerStr);
        printf("+++ CPU time  : %f\n", (float) (c_1 - c_0)/CLOCKS_PER_SEC);
        printf("+++ Wall time : %lf\n", seconds_timespecDiff(t_0, t_1));
	}
}

struct timespec timestamp()
{
	struct timespec timestampToReturn;

	clock_gettime(CLOCK_MONOTONIC, &timestampToReturn);

	return timestampToReturn;
}

#endif