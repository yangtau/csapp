#include <sys/mman.h>
#include <stddef.h>

#define LENGTH 8192

int main(void) {
	char *array = mmap(NULL, LENGTH, PROT_READ | PROT_WRITE, MAP_ANONYMOUS, -1, 0);

	// FIXME: I think there's a bug somewhere on the following line!
	for(size_t index = 0; index < LENGTH; ++index)
		array[index] = 'a' + index % 26;

	munmap(array, LENGTH);
	return 0;
}
