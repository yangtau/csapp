#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char RODATA[] = "RODATA";
static char data[] = "data";
static char bss[] = "";

static void print_addrs(FILE *stm, const char *heap) {
    char stack[] = "stack";
    fprintf(stm, "\t.text:   %#lx\n", (uintptr_t)print_addrs);
    fprintf(stm, "\t.rodata: %#lx\n", (uintptr_t)RODATA);
    fprintf(stm, "\t.data:   %#lx\n", (uintptr_t)data);
    fprintf(stm, "\t.bss:    %#lx\n", (uintptr_t)bss);
    fprintf(stm, "\theap:    %#lx\n", (uintptr_t)heap);
    fprintf(stm, "\tstack:   %#lx\n", (uintptr_t)stack);
}

int main(void) {
    char *heap = strdup("heap");
    if (!heap) {
        perror("strdup() failed to malloc() memory");
        return 1;
    }

    puts("Parent:");
    print_addrs(stdout, heap);
    int res = fork();
    if (res == 0) {
        puts("Child:");
        print_addrs(stdout, heap);
    } else {
        puts("Parent");
        printf("PID of child: %d\n", res);
    }

    free(heap);
    return 0;
}
