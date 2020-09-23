#include <stdio.h>
#include "slab.h"

/* Allocate a region with size PAGE_SIZE * 2^order bytes,
 * aligned at PAGE_SIZE * 2^order bytes. */
void *alloc_slab(int order)
{
	size_t size = PAGE_SIZE * (1 << order);
	return aligned_alloc(size, size);
}

/* Free SLAB memory. */
void free_slab(void *slab)
{
	free(slab);
}

/* set up the first free SLAB */
void initialize_new_free_slab(struct cache *cache)
{
	void *slab_header =
		GET_SLAB_HEADER(cache->free_slabs, cache->slab_order);
	GET_FREE_SLAB_CHUNK_AMOUNT(slab_header) = cache->slab_objects;
	GET_NEXT_SLAB(slab_header) = 0;
	GET_PREV_SLAB(slab_header) = 0;

	void *current_elem = slab_header;
	for (ssize_t i = cache->slab_objects - 1; i >= 0; i--) {
		GET_FREE_SLAB_CHUNK(current_elem) =
			(uintptr_t)(cache->free_slabs + i * cache->object_size);
		current_elem = (void *)GET_FREE_SLAB_CHUNK(current_elem);
	}
}

/* Allocate SLAB cache with objects given size. */
void cache_setup(struct cache *cache, size_t object_size)
{
	cache->object_size = object_size;
	if (cache->object_size < PTR_SIZE) {
		cache->object_size =
			PTR_SIZE; // because free chunks must contain pointers
	}

	cache->slab_order = cache->object_size / PAGE_SIZE;
	if (cache->object_size % PAGE_SIZE != 0 &&
	    cache->object_size > PAGE_SIZE) {
		cache->slab_order++;
	}
	if (cache->slab_order > MAX_PAGE_ORDER) {
		cache->slab_order = MAX_PAGE_ORDER;
	}
	cache->slab_objects =
		(PAGE_SIZE * (1 << cache->slab_order) - SLAB_HEADER_SIZE) /
		cache->object_size;
	cache->free_slabs = alloc_slab(cache->slab_order);
	cache->partly_allocated_slabs = NULL;
	cache->fully_allocated_slabs = NULL;

	initialize_new_free_slab(cache);
}

/* free chain of SLABs */
void free_slab_chain(void *chain_start, int order)
{
	while (1) {
		if (chain_start == NULL) {
			return;
		}
		void *next = (void *)GET_NEXT_SLAB(
			GET_SLAB_HEADER(chain_start, order));
		free_slab(chain_start);
		chain_start = next;
	}
}

/* Free SLAB cache. */
void cache_release(struct cache *cache)
{
	free_slab_chain(cache->free_slabs, cache->slab_order);
	cache->free_slabs = NULL;
	free_slab_chain(cache->partly_allocated_slabs, cache->slab_order);
	cache->partly_allocated_slabs = NULL;
	free_slab_chain(cache->fully_allocated_slabs, cache->slab_order);
	cache->fully_allocated_slabs = NULL;
}

/* delete SLAB from the list */
void delete_slab_from_list(void **list_beginning, void *slab_start, int order)
{
	void *header = GET_SLAB_HEADER(slab_start, order);
	void *next_slab = (void *)GET_NEXT_SLAB(header);
	void *prev_slab = (void *)GET_PREV_SLAB(header);
	if (next_slab != NULL) {
		GET_PREV_SLAB(GET_SLAB_HEADER(next_slab, order)) =
			(uintptr_t)prev_slab;
	}
	if (prev_slab != NULL) {
		GET_NEXT_SLAB(GET_SLAB_HEADER(prev_slab, order)) =
			(uintptr_t)next_slab;
	} else {
		*list_beginning = next_slab;
	}
}

/* insert SLAB at the beginning of the list */
void insert_slab_to_list(void **list_beginning, void *slab_start, int order)
{
	void *header = GET_SLAB_HEADER(slab_start, order);
	GET_NEXT_SLAB(header) = (uintptr_t)(*list_beginning);
	GET_PREV_SLAB(header) = 0;
	if (*list_beginning != NULL) {
		GET_PREV_SLAB(GET_SLAB_HEADER(*list_beginning, order)) =
			(uintptr_t)slab_start;
	}
	*list_beginning = slab_start;
}

/* Allocate new SLAB cache entry. */
void *cache_alloc(struct cache *cache)
{
	void *ret_chunk = NULL;

	if (cache->partly_allocated_slabs != NULL) {
		void *header = GET_SLAB_HEADER(cache->partly_allocated_slabs,
					       cache->slab_order);
		ret_chunk = (void *)GET_FREE_SLAB_CHUNK(header);
		GET_FREE_SLAB_CHUNK_AMOUNT(header)--;
		if (!GET_FREE_SLAB_CHUNK_AMOUNT(header)) {
			void *slab_start = cache->partly_allocated_slabs;
			delete_slab_from_list(&cache->partly_allocated_slabs,
					      slab_start, cache->slab_order);
			insert_slab_to_list(&cache->fully_allocated_slabs,
					    slab_start, cache->slab_order);
		} else {
			GET_FREE_SLAB_CHUNK(header) = GET_FREE_SLAB_CHUNK(
				GET_FREE_SLAB_CHUNK(header));
		}
	} else {
		if (cache->free_slabs == NULL) {
			cache->free_slabs = alloc_slab(cache->slab_order);
			initialize_new_free_slab(cache);
		}
		void *header =
			GET_SLAB_HEADER(cache->free_slabs, cache->slab_order);
		ret_chunk = (void *)GET_FREE_SLAB_CHUNK(header);
		GET_FREE_SLAB_CHUNK_AMOUNT(header)--;

		GET_FREE_SLAB_CHUNK(header) =
			GET_FREE_SLAB_CHUNK(GET_FREE_SLAB_CHUNK(header));

		void *slab_start = cache->free_slabs;
		delete_slab_from_list(&cache->free_slabs, slab_start,
				      cache->slab_order);
		insert_slab_to_list(&cache->partly_allocated_slabs, slab_start,
				    cache->slab_order);
	}

	return ret_chunk;
}

/* Return memory back to allocator. */
void cache_free(struct cache *cache, void *ptr)
{
	void *slab_start = (void *)GET_SLAB_START_PTR(ptr, cache->slab_order);
	void *header = (void *)GET_SLAB_HEADER(slab_start, cache->slab_order);

	GET_FREE_SLAB_CHUNK(ptr) = GET_FREE_SLAB_CHUNK(header);
	GET_FREE_SLAB_CHUNK(header) = (uintptr_t)ptr;
	GET_FREE_SLAB_CHUNK_AMOUNT(header)++;
	if (GET_FREE_SLAB_CHUNK_AMOUNT(header) == cache->slab_objects) {
		delete_slab_from_list(&cache->partly_allocated_slabs,
				      slab_start, cache->slab_order);
		insert_slab_to_list(&cache->free_slabs, slab_start,
				    cache->slab_order);
	} else if (GET_FREE_SLAB_CHUNK_AMOUNT(header) == 1) {
		delete_slab_from_list(&cache->fully_allocated_slabs, slab_start,
				      cache->slab_order);
		insert_slab_to_list(&cache->partly_allocated_slabs, slab_start,
				    cache->slab_order);
	}
}

/* Return free SLABs to system. */
void cache_shrink(struct cache *cache)
{
	free_slab_chain(cache->free_slabs, cache->slab_order);
	cache->free_slabs = NULL;
}
