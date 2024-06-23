#pragma once

#include <stddef.h>

void init_heap(void);
void *malloc(size_t size);
void *aligned_alloc(size_t alignment, size_t size);
void free(void *pointer);
void print_memory_blocks(void);

void *claim(void *address, size_t size, size_t alignment);
