#include <sys/mman.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static const uint8_t RET = 0xc3;

static int perms(const char *str) {
	int prot = 0;

	if(strchr(str, 'r'))
		prot |= PROT_READ;
	if(strchr(str, 'w'))
		prot |= PROT_WRITE;
	if(strchr(str, 'x'))
		prot |= PROT_EXEC;

	return prot;
}

static uint8_t *funchr(const uint8_t *f, uint8_t c) {
	long page = sysconf(_SC_PAGESIZE);
	return memchr(f, c, page - ((uintptr_t) f % page));
}

static bool nonzero(uint8_t val) {
	return !!val;
}

int main(int argc, char **argv) {
	if(argc != 2) {
		printf("USAGE: %s <permissions>\n"
			"<permissions> may be \"\" or any substring of: rwx\n",
			argv[0]);
		return 1;
	}

	uint8_t *func = (uint8_t *) (uintptr_t) nonzero;
	size_t size = funchr(func, RET) - func + 1;

	uint8_t *alloc = mmap(NULL, size, perms(argv[1]),
		MAP_ANONYMOUS | MAP_SHARED, -1, 0);
	if(alloc == MAP_FAILED) {
		perror("mmap()");
		return 2;
	}

	fputs("Reading... ", stdout);
	fflush(stdout);
	uint8_t res = *alloc;
	puts("success!");

	fputs("Writing... ", stdout);
	fflush(stdout);
	memcpy(alloc, func, size);
	puts("success!");

	fputs("Executing... ", stdout);
	fflush(stdout);
	bool (*nonzero)(uint8_t) = (bool (*)(uint8_t)) (uintptr_t) alloc;
	res = nonzero(res);
	puts("success!");

	munmap(alloc, size);
	return res;
}
