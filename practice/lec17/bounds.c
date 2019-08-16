#include "benchmark.h"

#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>

#define BYTES 102400

int main(void) {
    int8_t *two = mmap(NULL, BYTES, PROT_READ | PROT_WRITE,
                       MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (two == MAP_FAILED) {
        perror("mmap()");
        return 1;
    }

    for (size_t byte = 0; byte < BYTES; ++byte) {
        long pfs = faults();
        two[byte] = (int8_t)byte;
        if (faults() != pfs)
            printf("Page fault at offset %#7lx (%lu)\n", byte, byte);
    }

    munmap(two, BYTES);
    return 0;
}
