#pragma once

#include "string.h"

// loads the kernel into memory from the initrd tar file and starts it.
// the kernel is loaded into its own page table (this is architecture-specific) and invoked, meaning the MMU will be enabled when it starts
// if init_paging() is called beforehand, the behavior of this function is undefined
void load_kernel_from_tar(struct argument_pair *pairs, const char *initrd_start, const char *initrd_end);
