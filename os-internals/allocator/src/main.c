#include "allocator.h"
#include <stdio.h>
#include <time.h>

#define MAX_SIZE(size) ((size * 8) / 9)

int main() {
	size_t size = 0;
	int alloc_amount = 0;
	scanf("%lu %d", &size, &alloc_amount);

	srand(time(NULL));

	void *buf = malloc(size);
	mysetup(buf, size);

	char **array = malloc(sizeof(char*) * alloc_amount);
	for(int i = 0; i < alloc_amount; i++) {
		size_t random_size = rand() % size;
		array[i] = myalloc(random_size);
		if(!array[i]) {
			printf("ALLOCATED %lu bytes\n", random_size);
		}
	}

	for(int i = 0; i < alloc_amount; i++) {
		myfree(array[i]);
	}

	printf("ALLOCATE 8/9 of memory: ");
	void *memory = myalloc(MAX_SIZE(size));
	if(!memory) {
		printf("TEST FAILED!\n");
	} else {
		size_t number = rand();
		(*(size_t*)memory) = number;

		printf("SET: %lu, GET %lu\n", number, (*(size_t*)memory));
	}
	printf("FREE 8/9 of memory...");
	myfree(memory);
	return 0;
}
