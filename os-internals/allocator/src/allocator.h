#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stdlib.h>

#define ALLOCATED 1
#define NOT_ALLOCATED 0

#define MARKER_INFO_SIZE 1
#define SIZE_INFO_SIZE sizeof(size_t)
#define SPACE_BETWEEN_CHUNKS (MARKER_INFO_SIZE + 2 * SIZE_INFO_SIZE)

#define GET_MARKER(address)                                                    \
	(*((char *)address - (MARKER_INFO_SIZE + SIZE_INFO_SIZE)))
#define GET_SIZE(size_address) (*((size_t *)(size_address)))
#define GET_HEAD_SIZE(address) (address - SIZE_INFO_SIZE)
#define GET_TAIL_SIZE(address) (address + GET_SIZE(GET_HEAD_SIZE(address)))

#define GET_PREV_AREA(address)                                                 \
	(address - SPACE_BETWEEN_CHUNKS -                                      \
	 GET_SIZE((char *)address - SPACE_BETWEEN_CHUNKS))

#define GET_NEXT_AREA(address)                                                 \
	(address + SPACE_BETWEEN_CHUNKS + GET_SIZE(GET_HEAD_SIZE(address)))

extern void *logical_memory_area; // pointer to operating area
extern void *logical_memory_area_start; // start of operating area
extern size_t logical_memory_area_size; // size of area in bytes

/* setup allocator */
void mysetup(void *buf, size_t size);

/* allocate size bytes of memory */
void *myalloc(size_t size);

/* free memory */
void myfree(void *p);

#endif // ALLOCATOR_H
