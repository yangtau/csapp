#include <stdio.h>
#include <sys/mman.h>
#include <sys/sysinfo.h>

#define BYTES (1024 * 1024 * 1024)  // 1 GB

int main(void) {
    struct sysinfo inf;
    if (sysinfo(&inf)) {
        perror("sysinfo()");
        return 1;
    }
    printf("System has %lu GB of main memory\n",
           inf.totalram * inf.mem_unit / BYTES);

    for (int iter = 0;; ++iter)
        if (mmap(NULL, BYTES, 0, MAP_ANONYMOUS | MAP_SHARED, -1, 0) ==
            MAP_FAILED) {
            fprintf(stderr, "Failure after allocating %d GB\n", iter);
            perror("mmap()");
            return 1;
        }

    return 0;
}
