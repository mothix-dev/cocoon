#include "io.h"
#include "printf.h"
#include "version.h"
#include "multiboot.h"
#include "malloc.h"

void kmain(uint32_t signature, struct multiboot_header *header) {
    if (signature != 0x2badb002) {
        // signature is incorrect, don't bother proceeding with the boot process
        printf("FATAL: incorrect Multiboot signature 0x%x (expected 0x2badb002)\r\n", signature);
        while (1);
    }

    printf(NAME_VERSION_INFO "\r\n");
    printf("command line: \"%s\"\r\n", header->cmdline);

    if (header->mods_count == 0) {
        printf("FATAL: no modules found, cannot continue\r\n");
        while (1);
    }

    struct module_entry *initrd = header->mods_addr;

    printf("using module \"%s\" (entry at 0x%p, data from 0x%p to 0x%p) as initrd\r\n", initrd->string, initrd, initrd->start, initrd->end);

    printf("initrd contents: \"");
    for (char *c = (char *) initrd->start; c < (char *) initrd->end; c ++) {
        printf("%c", *c);
    }
    printf("\"\r\n");

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

    return address;
}
