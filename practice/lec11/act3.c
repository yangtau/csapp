#include <alloca.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void clobber(char*, int);
const char hiStr[] = "Hi\n";

void printAndExit(char* s) {
    puts(s);
    exit(0);
}

int main(int argc, char** argv) {
    char* x = alloca(48);
    unsigned char* m = malloc(128);

    puts("Activity 3!");
    if (m == NULL) {
        fprintf(stderr, "Allocation failure\n");
        return -1;
    }

    *(uint64_t*)m = (uint64_t)(0x4563ef); // pop %rdi; retq
    *(uint64_t*)(m + 8) = (uint64_t)(hiStr);
    *(uint64_t*)(m + 16) = (uint64_t)(0x415304); // pop %rax; retq
    *(uint64_t*)(m + 24) = (uint64_t)(&printAndExit);
    *(uint64_t*)(m + 32) = (uint64_t)(0x459008); // call *%rax
    clobber(m, 40);

    return 0;
}
