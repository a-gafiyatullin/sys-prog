#ifndef SLAB_H
#define SLAB_H

#include <stdlib.h>
#include <stdint.h>

#define PAGE_SIZE 4096 // 2^12
#define MAX_PAGE_ORDER 10
#define PTR_SIZE sizeof(void *)
#define CHUNKS_IN_SLAB_AMOUNT_SIZE (sizeof(size_t))
#define SLAB_HEADER_SIZE (PTR_SIZE * 3 + CHUNKS_IN_SLAB_AMOUNT_SIZE)

#define GET_NEXT_SLAB(slab_header)                                             \
	(*((uintptr_t *)(slab_header + PTR_SIZE + CHUNKS_IN_SLAB_AMOUNT_SIZE)))

#define GET_PREV_SLAB(slab_header)                                             \
	(*((uintptr_t *)(slab_header + 2 * PTR_SIZE +                          \
			 CHUNKS_IN_SLAB_AMOUNT_SIZE)))

#define GET_FREE_SLAB_CHUNK(slab_header) (*((uintptr_t *)(slab_header)))
#define GET_FREE_SLAB_CHUNK_AMOUNT(slab_header)                                \
	(*((size_t *)(slab_header + PTR_SIZE)))

#define GET_SLAB_HEADER(slab_start_ptr, order)                                 \
	(slab_start_ptr + PAGE_SIZE * (1 << order) - SLAB_HEADER_SIZE)

#define GET_SLAB_START_PTR(ptr, order)                                         \
	(void *)((size_t)ptr & (~((1 << (12 + order)) - 1)))

/* Allocate a region with size PAGE_SIZE * 2^order bytes,
 * aligned at PAGE_SIZE * 2^order bytes. */
void *alloc_slab(int order);

/* Free SLAB memory. */
void free_slab(void *slab);

/* SLAB cache information. */
struct cache {
	void *free_slabs;
	void *partly_allocated_slabs;
	void *fully_allocated_slabs;

	size_t object_size; /* allocated object size */
	int slab_order; /* SLAB order */
	size_t slab_objects; /* number of objects in SLAB */
};

/* Allocate SLAB cache with objects given size. */
void cache_setup(struct cache *cache, size_t object_size);

/* Free SLAB cache. */
void cache_release(struct cache *cache);

/* Allocate new SLAB cache entry. */
void *cache_alloc(struct cache *cache);

/* Return memory back to allocator. */
void cache_free(struct cache *cache, void *ptr);

/* Return free SLABs to system. */
void cache_shrink(struct cache *cache);

#endif // SLAB_H
