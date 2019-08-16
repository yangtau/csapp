#ifndef BENCHMARK_H_
#define BENCHMARK_H_

#include <sys/resource.h>
#include <sys/time.h>
#include <stddef.h>

static inline long time(void) {
	struct timeval stamp;
	int failure = gettimeofday(&stamp, NULL);
	return failure ? 0 : stamp.tv_sec * 1000000 + stamp.tv_usec;
}

static inline long faults(void) {
	struct rusage res;
	int failure = getrusage(RUSAGE_SELF, &res);
	return failure ? -1 : res.ru_minflt + res.ru_majflt;
}

#endif
