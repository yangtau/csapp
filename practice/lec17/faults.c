#include "benchmark.h"

#include <sys/mman.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BYTES 102400

int main(void) {
	puts("calloc():");
	long pfs = faults();
	int8_t *one = calloc(1, BYTES);
	printf("\tcall: %2ld page faults\n", faults() - pfs);
	if(one) {
		pfs = faults();
		for(size_t byte = 0; byte < BYTES; ++byte)
			one[byte] = byte;
		printf("\tloop: %2ld page faults\n", faults() - pfs);
		free(one);
	}
	putchar('\n');

	puts("mmap():");
	pfs = faults();
	int8_t *two = mmap(NULL, BYTES, PROT_READ | PROT_WRITE,
		MAP_ANONYMOUS | MAP_SHARED, -1, 0);
	printf("\tcall: %2ld page faults\n", faults() - pfs);
	if(two != MAP_FAILED) {
		pfs = faults();
		for(size_t byte = 0; byte < BYTES; ++byte)
			two[byte] = byte;
		printf("\tloop: %2ld page faults\n", faults() - pfs);
		munmap(two, BYTES);
	}

	return 0;
}
