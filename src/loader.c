#include "loader.h"
#include <elf.h> /* TODO: is this portable? */
#include <stdint.h>
#include "platform/bsp.h"
#include "malloc.h"
#include "tar.h"
#include "printf.h"

#if BITS == 32
typedef Elf32_Ehdr Elf_Ehdr;
typedef Elf32_Phdr Elf_Phdr;
#define BIT_CLASS 1
#elif BITS == 64
typedef Elf64_Ehdr Elf_Ehdr;
typedef Elf64_Phdr Elf_Phdr;
#define BIT_CLASS 2
#else
#error invalid bit width, only 32 or 64 bit is supported
#endif

void load_kernel_from_tar(struct argument_pair *pairs, const char *initrd_start, const char *initrd_end) {
    //printf("initrd is at 0x%p to 0x%p (size %d)\r\n", initrd_start, initrd_end, initrd_end - initrd_start);

    const char *kernel_filename = "/actias";
    for (struct argument_pair *p = pairs; p->key != NULL; p ++)
        if (strcmp(p->key, "kernel") == 0 && p->value != NULL) {
            kernel_filename = p->value;
            break;
        }

    printf("using \"%s\" as kernel\r\n", kernel_filename);

    const char *data;
    size_t size;

    struct tar_iterator *initrd = open_tar(initrd_start, initrd_end);
    if (!tar_find(initrd, kernel_filename, TAR_NORMAL_FILE, &data, &size)) {
        printf("FATAL: couldn't find kernel in initrd\r\n");
        while (1);
    }

    if (size < sizeof(Elf_Ehdr)) {
        printf("FATAL: kernel is too small\r\n");
        while (1);
    }

    //printf("kernel is at 0x%p to 0x%p (size %d)\r\n", data, data + size, size);

    //print_memory_blocks();

    init_paging();

    const uint8_t *ident = (uint8_t *) data;

    // initial sanity checks to make sure the kernel probably isn't horribly fucked up
    if (
        ident[EI_MAG0] != ELFMAG0
        || ident[EI_MAG1] != ELFMAG1
        || ident[EI_MAG2] != ELFMAG2
        || ident[EI_MAG3] != ELFMAG3
    ) {
        printf("FATAL: kernel's magic number is invalid\r\n");
        while (1);
    }

    if (ident[EI_CLASS] != BIT_CLASS) {
        switch (ident[EI_CLASS]) {
            case 1:
                printf("FATAL: kernel is 32 bit, expected 64 bit\r\n");
                break;
            case 2:
                printf("FATAL: kernel is 64 bit, expected 32 bit\r\n");
                break;
            default:
                printf("FATAL: kernel bit width is unknown\r\n");
        }
        while (1);
    }

    if (ident[EI_VERSION] != EV_CURRENT) {
        printf("FATAL: kernel is ELF version %d, expected version %d\r\n", ident[EI_VERSION], EV_CURRENT);
        while (1);
    }

    const Elf_Ehdr *header = (const Elf_Ehdr *) data;

    if (header->e_version != EV_CURRENT) {
        printf("FATAL: kernel is ELF version %d, expected version %d (maybe wrong endianness?)\r\n", header->e_version, EV_CURRENT);
        while (1);
    }

    void *program_headers = (void *) (data + header->e_phoff);
    size_t ph_size = (size_t) header->e_phentsize;
    size_t ph_count = (size_t) header->e_phnum;

    printf("%d %d-byte program headers at 0x%p\r\n", ph_count, ph_size, program_headers);

    for (int i = 0; i < ph_count; i ++) {
        const Elf_Phdr *program_header = (const Elf_Phdr *) (program_headers + i * ph_size);

        if (program_header->p_type != PT_LOAD) continue;
        if (program_header->p_memsz == 0) continue;

        const char *data_in_file = data + program_header->p_offset;

        if (program_header->p_filesz > 0 && (data_in_file >= initrd_end || data_in_file + program_header->p_filesz >= initrd_end)) {
            printf("FATAL: program header's data is out of bounds\r\n");
            while (1);
        }

        size_t page_aligned_start = (size_t) program_header->p_vaddr & ~(PAGE_SIZE - 1);
        size_t page_aligned_end = ((size_t) (program_header->p_vaddr + program_header->p_memsz) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

        size_t offset_start = (size_t) program_header->p_vaddr - page_aligned_start;
        size_t end_addr = program_header->p_vaddr + program_header->p_filesz;

        /*printf(
            "load 0x%p to 0x%p (page aligned 0x%p to 0x%p) from 0x%p to 0x%p\r\n",
            program_header->p_vaddr,
            program_header->p_vaddr + program_header->p_memsz,
            page_aligned_start,
            page_aligned_end,
            data_in_file,
            data_in_file + program_header->p_filesz
        );*/

        for (size_t page_address = page_aligned_start; page_address < page_aligned_end; page_address += PAGE_SIZE) {
            void *mapped_address = get_mapped_address_for(page_address);

            if (mapped_address == NULL) {
                printf("FATAL: failed to allocate memory for kernel\r\n");
                while (1);
            }

            if (page_address > end_addr) continue;

            size_t start_offset = page_address < program_header->p_vaddr ? program_header->p_vaddr - page_address : 0;
            size_t end_offset = page_address + PAGE_SIZE > end_addr ? end_addr - page_address : PAGE_SIZE;

            const char *src = data_in_file - offset_start + page_address - page_aligned_start + start_offset;
            char *dest = (char *) mapped_address + start_offset;

            //printf("copying from 0x%p to 0x%p (start offset 0x%x, end offset 0x%x, %d bytes)\r\n", src, dest, start_offset, end_offset, end_offset - start_offset);
            for (size_t i = start_offset; i < end_offset; i ++) {
                *(dest ++) = *(src ++);
            }
        }
    }

    //print_memory_blocks();

    // TODO: map in initrd, memory map, device tree, etc

    size_t kernel_entry_point = (size_t) header->e_entry;
    printf("starting kernel at 0x%p...\r\n", kernel_entry_point);
    transfer_control(kernel_entry_point);
}
