#pragma once

#include <stddef.h>

// initializes the heap so that allocations can be made
void init_heap(void);

// allocates an unaligned region of memory of the specified size and returns a pointer to it
void *malloc(size_t size);

// allocates a region of memory aligned to the specified alignment and returns a pointer to it
// alignment must be either 0 or a power of 2, the behavior is undefined otherwise
void *aligned_alloc(size_t alignment, size_t size);

// frees a region of memory pointed to by the given pointer so that it can be reused for future allocations
// if the pointer doesn't point to the start of a region of memory, the behavior is undefined
void free(void *pointer);

// prints out a list of all the used/free memory blocks for debugging purposes
void print_memory_blocks(void);

// claims a region of memory for use by the heap. this is platform-specific and must be implemented accordingly
// the behavior should be that of OpenFirmware's claim function
void *claim(void *address, size_t size, size_t alignment);
