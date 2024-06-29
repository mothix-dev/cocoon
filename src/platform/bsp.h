#pragma once

#include <stdint.h>
#include <stddef.h>

// === architecture-specific functions (independent of platform) ===

// initializes the page table that the kernel will live in, so that individual pages can be mapped into it
// if this function is called multiple times, its behavior is undefined
void init_paging(void);

// gets the mapped address for a page in the kernel's page table, allocating a new page for it in the process if required
// if this fails, a null pointer must be returned
void *get_mapped_address_for(size_t virtual_for_kernel);

// transfers control to the kernel in its page table at the specified entry point
// if the MMU isn't enabled then it should be enabled and the kernel's page table should be swapped in
void transfer_control(size_t kernel_entry_point);

// === platform-specific functions (dependent on both architecture and platform) ===

// translates a virtual address to a physical one, in case the MMU is enabled before control is handed off to the kernel
// if no MMU is active, it should just return the address unchanged
uint64_t virtual_address_to_physical(void *address);
