#include <stdio.h>

int main() {
    unsigned x = 0xffff;
    x >>= 8;
    printf("%x\n", x);
    unsigned y = ~0;
    printf("%x\n", y);
    unsigned long z = ~0;
    printf("%lx\n", z);
    printf("%lx\n", z >> (64 - 8));
    return 0;
}