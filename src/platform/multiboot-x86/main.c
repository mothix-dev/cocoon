#include "io.h"
#include "printf.h"
#include "version.h"
#include "multiboot.h"
#include "malloc.h"
#include "string.h"
#include "tar.h"
#include "loader.h"
#include "platform/bsp.h"

struct {
    struct multiboot_header *header;
    size_t cmdline_length;
    struct module_entry initrd_module;
} claim_state;

void kmain(uint32_t signature, struct multiboot_header *header) {
    if (signature != 0x2badb002) {
        // signature is incorrect, don't bother proceeding with the boot process
        printf("FATAL: incorrect Multiboot signature 0x%x (expected 0x2badb002)\r\n", signature);
        while (1);
    }

    printf(NAME_VERSION_INFO "\r\n");

    printf("command line: \"%s\"\r\n", header->cmdline);
    size_t cmdline_length = strlen(header->cmdline);

    if (header->mods_count == 0) {
        printf("FATAL: no modules found, cannot continue\r\n");
        while (1);
    }

    struct module_entry initrd_module = *header->mods_addr;

    printf("using module \"%s\" (entry at 0x%p, data from 0x%p to 0x%p) as initrd\r\n", initrd_module.string, header->mods_addr, initrd_module.start, initrd_module.end);

    claim_state.header = header;
    claim_state.cmdline_length = cmdline_length;
    claim_state.initrd_module = initrd_module;

    init_heap();
    struct argument_pair *pairs = parse_arguments(header->cmdline);
    if (pairs == NULL) {
        printf("FATAL: couldn't allocate memory for parsed command line\r\n");
        while (1);
    }

    load_kernel_from_tar(pairs, initrd_module.start, initrd_module.end);

    while (1);
}

void _putchar(char c) {
    // Wait for the serial port's fifo to not be empty
    while ((inb(0x3F8 + 5) & 0x20) == 0);
    // Send the byte out the serial port
    outb(0x3F8, c);

    // Also send to the bochs 0xe9 hack
    outb(0xe9, c);
}

void *claim(void *address, size_t size, size_t alignment) {
    if (alignment != 0) {
        printf("TODO: claim alignment\r\n");
        return NULL;
    }

    void *end = address + size;

    void *header_start = (void *) claim_state.header;
    void *header_end = header_start + sizeof(struct multiboot_header);
    if (end >= header_start && header_end >= address) {
        return NULL;
    }

    void *cmdline_start = (void *) claim_state.header->cmdline;
    void *cmdline_end = cmdline_start + claim_state.cmdline_length;
    if (end >= cmdline_start && cmdline_end >= address) {
        return NULL;
    }

    if (end >= claim_state.initrd_module.start && claim_state.initrd_module.end >= address) {
        return NULL;
    }

    // TODO: check memory map to make sure this claim isn't in an unavailable memory region

    return address;
}

uint64_t virtual_address_to_physical(void *address) {
    return (uint64_t) address;
}
