#include <stdio.h>
#include <time.h>
#include "slab.h"

int main()
{
	srand(time(NULL));

	struct cache cache20;
	cache_setup(&cache20, 20);

	void *memory20_1 = cache_alloc(&cache20);
	*((long long int *)memory20_1) = 201;

	void *memory20_2 = cache_alloc(&cache20);
	*((long long int *)memory20_2) = 301;
	printf("TEST VALUE: %lld\n", *((long long int *)memory20_1));
	printf("TEST VALUE: %lld\n", *((long long int *)memory20_2));

	size_t chunks_num = cache20.slab_objects * 4;
	void **array = malloc(sizeof(void *) * chunks_num);
	for (size_t i = 0; i < chunks_num; i++) {
		array[i] = cache_alloc(&cache20);
		*((long long int *)array[i]) = i;
	}

	for (ssize_t i = 0; i < chunks_num; i++) {
		printf("TEST VALUE: %lld ", *((long long int *)array[i]));
		printf("TEST VALUE: %p\n", array[i]);
	}

	for (size_t i = 0; i < cache20.slab_objects * 2; i++) {
		cache_free(&cache20, array[i]);
	}

	for (size_t i = 0; i < cache20.slab_objects; i++) {
		array[i] = cache_alloc(&cache20);
	}

	cache_shrink(&cache20);

	size_t chunk_num_to_free =
		rand() % cache20.slab_objects + cache20.slab_objects;
	printf("Try to free chunk %zu...\n", chunk_num_to_free);
	cache_free(&cache20, array[chunk_num_to_free]);

	chunk_num_to_free =
		rand() % cache20.slab_objects + 2 * cache20.slab_objects;
	printf("Try to free chunk %zu...\n", chunk_num_to_free);
	cache_free(&cache20, array[chunk_num_to_free]);

	chunk_num_to_free =
		rand() % cache20.slab_objects + 3 * cache20.slab_objects;
	printf("Try to free chunk %zu...\n", chunk_num_to_free);
	cache_free(&cache20, array[chunk_num_to_free]);

	cache_release(&cache20);
	return 0;
}
