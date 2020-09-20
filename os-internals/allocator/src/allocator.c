#include "allocator.h"

void *logical_memory_area = NULL; // pointer to operating area
void *logical_memory_area_start = NULL; // start of operating area
size_t logical_memory_area_size = 0; // size of area in bytes

/* setup allocator */
void mysetup(void *buf, size_t size)
{
	logical_memory_area_size = size;
	logical_memory_area = buf;
	logical_memory_area_start = buf + (MARKER_INFO_SIZE + SIZE_INFO_SIZE);
	GET_MARKER(logical_memory_area_start) = NOT_ALLOCATED;
	GET_SIZE(GET_HEAD_SIZE(logical_memory_area_start)) =
		logical_memory_area_size - SPACE_BETWEEN_CHUNKS;
	GET_SIZE(GET_TAIL_SIZE(logical_memory_area_start)) =
		logical_memory_area_size - SPACE_BETWEEN_CHUNKS;
}

/* allocate size bytes of memory */
void *myalloc(size_t size)
{
	size_t curr_total_size = (MARKER_INFO_SIZE + SIZE_INFO_SIZE);
	void *area_start = logical_memory_area_start;
	void *ret_ptr = NULL;

	/* search for not allocated region */
	while (curr_total_size < logical_memory_area_size) {
		ssize_t current_area_size = GET_SIZE(GET_HEAD_SIZE(area_start));
		/* if region not allocated and enough memory */
		if (GET_MARKER(area_start) == NOT_ALLOCATED &&
		    current_area_size >= size) {
			ret_ptr = area_start;
			/* set region as allocated */
			GET_MARKER(area_start) = ALLOCATED;
			/* if chunk is greater than requested memory size */
			ssize_t new_size = current_area_size - (ssize_t)size -
					   SPACE_BETWEEN_CHUNKS;
			if (new_size > 0) {
				/* set new size for the next chunk */
				GET_SIZE(GET_TAIL_SIZE(area_start)) = new_size;
				GET_SIZE(GET_HEAD_SIZE(area_start + size +
						       SPACE_BETWEEN_CHUNKS)) =
					new_size;
				/* set next chunk as not allocated */
				GET_MARKER(area_start + size +
					   SPACE_BETWEEN_CHUNKS) =
					NOT_ALLOCATED;
				/* set new size for allocated chunk */
				GET_SIZE(GET_HEAD_SIZE(area_start)) = size;
				GET_SIZE(GET_TAIL_SIZE(area_start)) = size;
			}
			break;
		} else {
			curr_total_size = curr_total_size + current_area_size +
					  SPACE_BETWEEN_CHUNKS;
			area_start = logical_memory_area + curr_total_size;
		}
	}

	return ret_ptr;
}

/* free size bytes of memory */
void myfree(void *p)
{
	if (p == NULL) {
		return;
	}
	if (GET_MARKER(p) == NOT_ALLOCATED) {
		return;
	}

	void *current_ptr = p;
	while (1) {
		char prev_marker = -1, next_marker = -1;
		void *prev_area = NULL;
		void *next_area = NULL;

		/* check if it is the first chunk */
		if (current_ptr - (MARKER_INFO_SIZE + SIZE_INFO_SIZE) >
		    logical_memory_area) {
			prev_area = GET_PREV_AREA(current_ptr);
			prev_marker = GET_MARKER(prev_area);
		}
		/* check if it is the last chunk */
		if (current_ptr + GET_SIZE(GET_TAIL_SIZE(current_ptr)) +
			    SIZE_INFO_SIZE <
		    logical_memory_area + logical_memory_area_size) {
			next_area = GET_NEXT_AREA(current_ptr);
			next_marker = GET_MARKER(next_area);
		}
		/* check if can't do defragmentation */
		if ((prev_marker == ALLOCATED || prev_marker == -1) &&
		    (next_marker == ALLOCATED || next_marker == -1)) {
			GET_MARKER(current_ptr) = NOT_ALLOCATED;
			return;
		}
		/* connect the previous chunk */
		if (prev_marker == NOT_ALLOCATED) {
			GET_SIZE(GET_HEAD_SIZE(prev_area)) =
				GET_SIZE(GET_HEAD_SIZE(prev_area)) +
				GET_SIZE(GET_HEAD_SIZE(current_ptr)) +
				SPACE_BETWEEN_CHUNKS;
			GET_SIZE(GET_TAIL_SIZE(current_ptr)) =
				GET_SIZE(GET_HEAD_SIZE(prev_area));
			GET_MARKER(prev_area) = ALLOCATED;
			current_ptr = prev_area;
		}
		/* connect the next chunk */
		if (next_marker == NOT_ALLOCATED) {
			GET_SIZE(GET_HEAD_SIZE(current_ptr)) =
				GET_SIZE(GET_HEAD_SIZE(current_ptr)) +
				GET_SIZE(GET_HEAD_SIZE(next_area)) +
				SPACE_BETWEEN_CHUNKS;
			GET_SIZE(GET_TAIL_SIZE(next_area)) =
				GET_SIZE(GET_HEAD_SIZE(current_ptr));
			GET_MARKER(current_ptr) = ALLOCATED;
		}
	}
}
