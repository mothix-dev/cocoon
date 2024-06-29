#pragma once

#include <stdint.h>
#include <stddef.h>

// architecture-specific functions (independent of platform)

void init_paging(void);
void *get_mapped_address_for(size_t virtual_for_kernel);
void transfer_control(size_t kernel_entry_point);

// platform-specific functions (dependent on both architecture and platform)

uint64_t virtual_address_to_physical(void *address);
