#include <alloca.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

void clobber(char*, int);
const char hiStr[] = "Hi\n";

int main(int argc, char** argv) {
    char* x = alloca(32);
    unsigned char* m = malloc(128);

    puts("Activity 2!");
    if (m == NULL) {
        fprintf(stderr, "Allocation failure\n");
        return -1;
    }
    if (mprotect((void*)(((uint64_t)x) & (~0xfff)), 4096,
                 PROT_READ | PROT_WRITE | PROT_EXEC) == -1) {
        perror("MPROTECT");
        free(m);
        return -1;
    }
    *(uint64_t*)m = (uint64_t)(x);
    m[8] = 0xbf;
    *(uint32_t*)(m + 9) = (unsigned int)(uint64_t)hiStr;
    *(uint32_t*)(m + 13) = 0x410790be; // mov $0x4022e0 %esi
    *(uint32_t*)(m + 18) = 0xd6ff;     // call *%rsi
    *(uint32_t*)(m + 20) = 0x40eb30be; // mov $0x4011a0 %esi
    *(uint32_t*)(m + 25) = 0xd6ff;     // call *%rsi
    clobber(m, 32);
    // exit 0x40eb30
    // puts 0x410790
    return 0;
}