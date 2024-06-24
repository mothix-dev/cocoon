#include "malloc.h"
#include <stdbool.h>
#include <stddef.h>
#include "printf.h"

struct header {
    // the size of this memory block, including the header and footer
    size_t block_size;
    // whether this block is free and can be allocated in
    bool is_free;
};

struct footer {
    // a pointer to the header associated with this footer
    struct header *header;
};

extern char _end;
struct header *heap_start = NULL;
struct header *original_heap_start = NULL;
void *heap_end = NULL;

static void expand_around(struct header *header);

void init_heap(void) {
    heap_start = original_heap_start = heap_end = (struct header *) (((size_t) &_end + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1));
}

void *malloc(size_t size) {
    return aligned_alloc(1, size);
}

void *aligned_alloc(size_t alignment, size_t size) {
    if (alignment == 0) alignment = 1;
    if (size == 0) return NULL;

    struct header *header = heap_start;
    size_t aligned_data_address;

    // find a free memory block that's big enough to fit this allocation
    for (; (void *) header < heap_end; header = (struct header *) ((size_t) header + header->block_size)) {
        if (!header->is_free) continue;

        aligned_data_address = ((size_t) header + sizeof(struct header) + alignment - 1) & ~(alignment - 1);
        if (aligned_data_address + size >= (size_t) header + header->block_size - sizeof(struct footer)) continue;

        // found a valid header!
        break;
    }

    if ((void *) header >= heap_end) {
        // allocate more memory and make a valid header and footer for this new block, then try allocating again
        // if the last memory block is free, expand it without creating a new block

        size_t aligned_end = ((size_t) heap_end + alignment - 1) & ~(alignment - 1);
        void *new_heap_end = (void *) ((aligned_end + size + sizeof(struct header) + sizeof(struct footer) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1));

        size_t claim_size = (size_t) (new_heap_end - heap_end);

        while (1) {
            printf("malloc: claiming 0x%p to 0x%p (size %d)\r\n", heap_end, new_heap_end, claim_size);

            void *claimed_address = claim(heap_end, claim_size, 0);
            if (claimed_address == NULL || claimed_address != heap_end) {
                if (heap_start == heap_end && ((size_t) heap_start) < (size_t) (-PAGE_SIZE)) {
                    // the heap hasn't been set up yet, find the first area of available memory and move the start of the heap to it
                    printf("malloc: claim failed, trying again\r\n");

                    original_heap_start = heap_start = heap_end = (struct header *) ((size_t) heap_start + PAGE_SIZE);
                    new_heap_end += PAGE_SIZE;

                    continue;
                } else {
                    printf("malloc: claim failed (out of memory?)\r\n");
                    return NULL;
                }
            }

            break;
        }

        struct header *new_header = (struct header *) heap_end;
        struct footer *new_footer = (struct footer *) ((size_t) new_heap_end - sizeof(struct footer));

        new_header->is_free = true;
        new_header->block_size = claim_size;
        new_footer->header = new_header;

        heap_end = new_heap_end;

        expand_around(new_header);

        return aligned_alloc(alignment, size);
    }

    struct header *aligned_header = (struct header *) (aligned_data_address - sizeof(struct header));
    struct footer *footer = (struct footer *) ((size_t) header + header->block_size - sizeof(struct footer));

    // make sure the aligned header and footer are correct
    aligned_header->block_size = (size_t) footer - (size_t) aligned_header + sizeof(struct footer);
    aligned_header->is_free = false;
    footer->header = aligned_header;

    // check if the header has to be moved, and either create a new memory block or expand the previous memory block to fit
    size_t difference = (size_t) aligned_header - (size_t) header;

    if (difference > 0 && difference <= sizeof(struct header) + sizeof(struct footer)) {
        // there isn't enough space to make a new memory block, so the block below this one should be expanded

        struct footer *previous_footer = (struct footer *) ((size_t) header - sizeof(struct footer));

        if ((size_t) previous_footer > (size_t) original_heap_start + sizeof(struct header)) {
            struct header *previous_header = previous_footer->header;
            struct footer *new_footer = (struct footer *) ((size_t) aligned_header - sizeof(struct footer));

            if (header == heap_start) {
                heap_start = original_heap_start;

                heap_start->is_free = true;
                heap_start->block_size = (size_t) aligned_header - (size_t) heap_start;
            } else {
                new_footer->header = previous_header;
                previous_header->block_size = (size_t) aligned_header - (size_t) previous_header;
            }
        } else {
            // there isn't enough room to fit a new memory block between the start of the heap and this memory block,
            // so the start of the heap has to be increased to the start of this new memory block
            heap_start = aligned_header;
        }
    } else if (difference > 0) {
        struct footer *new_footer = (struct footer *) ((size_t) aligned_header - sizeof(struct footer));
        new_footer->header = header;
        header->block_size = difference;

        expand_around(header);
    }

    // split end of block if there's enough room
    struct footer *shrunk_footer = (struct footer *) (aligned_data_address + size);
    difference = (size_t) footer - (size_t) shrunk_footer;

    if (difference > sizeof(struct header) + sizeof(struct footer)) {
        struct header *new_header = (struct header *) ((size_t) shrunk_footer + sizeof(struct footer));

        new_header->is_free = true;
        new_header->block_size = (size_t) footer - (size_t) new_header + sizeof(struct footer);
        footer->header = new_header;

        aligned_header->block_size = size + sizeof(struct header) + sizeof(struct footer);
        shrunk_footer->header = aligned_header;

        expand_around(new_header);
    }

    return (void *) ((size_t) aligned_header + sizeof(struct header));
}

static void expand_around(struct header *header) {
    struct footer *footer = (struct footer *) ((size_t) header + header->block_size - sizeof(struct footer));

    if (footer->header != header) {
        printf("FATAL: malloc/free: footer's pointer to header doesn't match!\r\n");
        while (1);
    }

    header->is_free = true;

    struct header *next_header = (struct header *) ((size_t) header + header->block_size);

    if ((void *) next_header < heap_end && next_header->is_free) {
        // combine with next block

        footer = (struct footer *) ((size_t) next_header + next_header->block_size - sizeof(struct footer));

        footer->header = header;
        header->block_size += next_header->block_size;
    }

    struct footer *previous_footer = (struct footer *) ((size_t) header - sizeof(struct footer));
    struct header *previous_header;

    if ((struct header *) previous_footer > heap_start && (previous_header = previous_footer->header)->is_free) {
        // combine with previous block

        footer->header = previous_header;
        previous_header->block_size += header->block_size;
    }
}

void free(void *pointer) {
    if (pointer == NULL || pointer < (void *) heap_start + sizeof(struct header) || pointer >= heap_end) return;

    struct header *header = (struct header *) ((size_t) pointer - sizeof(struct header));
    expand_around(header);
}

void print_memory_blocks(void) {
    printf("memory blocks list:\r\n");

    if (heap_start == heap_end) {
        printf("    no blocks in heap\r\n");
        return;
    }

    struct header *header = heap_start;
    for (; (void *) header < heap_end; header = (struct header *) ((size_t) header + header->block_size)) {
        size_t data_size = header->block_size - sizeof(struct header) - sizeof(struct footer);
        printf("    0x%p: %s, data size %d (0x%x), block size %d (0x%x)\r\n", header, header->is_free ? "free" : "used", data_size, data_size, header->block_size, header->block_size);

        struct footer *footer = (struct footer *) ((size_t) header + header->block_size - sizeof(struct footer));

        if (footer->header != header) {
            printf("FATAL: malloc/free: footer's pointer to header doesn't match!\r\n");
            while (1);
        }
    }
}
