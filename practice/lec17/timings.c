#include "benchmark.h"

#include <sys/mman.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BYTES 102400

int main(void) {
	puts("calloc():");
	long call, loop = 0;
	long ts = time();
	int8_t *one = calloc(1, BYTES);
	call = time() - ts;
	printf("\tcall: %3ld microseconds\n", call);
	if(one) {
		ts = time();
		for(size_t byte = 0; byte < BYTES; ++byte)
			one[byte] = (int8_t) byte;
		loop = time() - ts;
		printf("\tloop: %3ld microseconds\n", loop);
		free(one);
	}
	printf("\tsum:  %3ld microseconds\n", call + loop);
	putchar('\n');

	puts("mmap():");
	ts = time();
	int8_t *two = mmap(NULL, BYTES, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_SHARED, -1, 0);
	call = time() - ts;
	printf("\tcall: %3ld microseconds\n", call);
	if(two != MAP_FAILED) {
		long ts = time();
		for(size_t byte = 0; byte < BYTES; ++byte)
			two[byte] = (int8_t) byte;
		loop = time() - ts;
		printf("\tloop: %3ld microseconds\n", loop);
		munmap(two, BYTES);
	}
	printf("\tsum:  %3ld microseconds\n", call + loop);

	return 0;
}
