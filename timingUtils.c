#ifndef TIMING_UTILS
#define	TIMING_UTILS

#include <time.h>
#include <stdint.h>


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

struct timespec timestamp()
{
	struct timespec timestampToReturn;

	clock_gettime(CLOCK_MONOTONIC, &timestampToReturn);

	return timestampToReturn;
}

#endif