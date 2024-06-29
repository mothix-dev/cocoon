#include <stdint.h>
#include "platform/bsp.h"
#include "printf.h"
#include "malloc.h"

// TODO: PAE support

struct page {
    uint32_t present: 1;
    uint32_t read_write: 1;
    uint32_t user_mode: 1;
    uint32_t was_accessed: 1;
    uint32_t is_dirty: 1;
    uint32_t unused: 7;
    uint32_t frame: 20;
};

struct page_table {
    struct page pages[1024];
    void *mapped_virtual_addresses[1024];
};

struct page_directory {
    uint32_t physical_page_tables[1024];
    uint32_t physical_address;
    struct page_table *page_tables[1024];
};

struct page_directory *the_page_directory;

uint32_t checked_virtual_to_physical(void *address) {
    uint64_t physical_address = virtual_address_to_physical(address);

    if ((physical_address & ~0xffffffff) != 0) {
        printf("FATAL: invalid physical address 0x%x for virtual address 0x%p\r\n", physical_address, address);
        while (1);
    }

    return (uint32_t) physical_address;
}

void init_paging(void) {
    the_page_directory = aligned_alloc(PAGE_SIZE, sizeof(struct page_directory));
    if (the_page_directory == NULL) {
        printf("FATAL: failed to allocate memory for the kernel's page directory\r\n");
        while (1);
    }

    for (int i = 0; i < 1024; i ++) {
        the_page_directory->physical_page_tables[i] = 0;
        the_page_directory->page_tables[i] = NULL;
    }

    the_page_directory->physical_address = checked_virtual_to_physical((void *) the_page_directory);

    printf("allocated page directory at 0x%p (physical 0x%p)\r\n", the_page_directory, the_page_directory->physical_address);
}

struct page_table *get_page_table(size_t address) {
    address /= PAGE_SIZE;

    size_t table_index = address >> 10;
    if (the_page_directory->page_tables[table_index] != NULL) {
        return the_page_directory->page_tables[table_index];
    } else {
        struct page_table *new_page_table = aligned_alloc(PAGE_SIZE, sizeof(struct page_table));
        if (new_page_table == NULL) {
            printf("FATAL: failed to allocate memory for a page table\r\n");
            while (1);
        }

        for (int i = 0; i < 1024; i ++) {
            new_page_table->mapped_virtual_addresses[i] = NULL;
            new_page_table->pages[i].present = 0;
        }

        uint32_t physical_address = checked_virtual_to_physical((void *) new_page_table);

        the_page_directory->page_tables[table_index] = new_page_table;
        the_page_directory->physical_page_tables[table_index] = physical_address | 0x7; // present, r/w, user mode

        //printf("allocated page table for 0x%p at 0x%p (physical 0x%p)\r\n", address * PAGE_SIZE, new_page_table, physical_address);

        return new_page_table;
    }
}

void *allocate_zeroed_page(void) {
    void *page = aligned_alloc(PAGE_SIZE, PAGE_SIZE);
    if (page == NULL) {
        printf("FATAL: failed to allocate memory for kernel\r\n");
        while (1);
    }

    // zero out newly allocated page just in case
    size_t *data = (size_t *) page;
    for (int i = 0; i < PAGE_SIZE / sizeof(size_t); i ++) {
        *(data ++) = 0;
    }

    return page;
}

void *get_mapped_address_for(size_t address) {
    struct page_table *page_table = get_page_table(address);

    size_t sub_index = (address / PAGE_SIZE) & 1023;
    void *mapped_virtual_address = page_table->mapped_virtual_addresses[sub_index];

    if (mapped_virtual_address == NULL) {
        mapped_virtual_address = allocate_zeroed_page();
        uint32_t physical_address = checked_virtual_to_physical(mapped_virtual_address);

        page_table->mapped_virtual_addresses[sub_index] = mapped_virtual_address;

        struct page *page = &page_table->pages[sub_index];
        page->present = 1;
        page->read_write = 1;
        page->user_mode = 0;
        page->frame = (uint32_t) physical_address / PAGE_SIZE;

        //printf("allocated page for 0x%p at 0x%p (physical 0x%p)\r\n", address * PAGE_SIZE, mapped_virtual_address, physical_address);
    }

    return mapped_virtual_address;
}

const uint8_t trampoline_data[] = {
    0xb8, 0x00, 0x00, 0x00, 0x00,   // mov $0, %eax
    0x0f, 0x22, 0xd8,               // mov %eax, %cr3
    0x0f, 0x20, 0xc0,               // mov %cr0, %eax
    0x0d, 0x00, 0x00, 0x01, 0x80,   // or $0x80010000, %eax /* PG & WP */
    0x0f, 0x22, 0xc0,               // mov %eax, %cr0
    0xb8, 0x00, 0x00, 0x00, 0x00,   // mov $0, %eax
    0xff, 0xe0                      // jmp %eax
};

void transfer_control(size_t kernel_entry_point) {
    uint8_t *trampoline_page = aligned_alloc(PAGE_SIZE, PAGE_SIZE);
    if (trampoline_page == NULL) {
        printf("FATAL: failed to allocate memory for trampoline\r\n");
        while (1);
    }

    // map trampoline page into the kernel's memory at the same address it's currently mapped in at
    size_t address = (size_t) trampoline_page;
    struct page_table *page_table = get_page_table(address);
    size_t sub_index = (address / PAGE_SIZE) & 1023;

    if (page_table->mapped_virtual_addresses[sub_index] != NULL) {
        printf("FATAL: page is already mapped at 0x%p, can't map trampoline in\r\n", trampoline_page);
        while (1);
    }

    uint32_t physical_address = checked_virtual_to_physical((void *) trampoline_page);

    struct page *page = &page_table->pages[sub_index];
    page->present = 1;
    page->read_write = 1;
    page->user_mode = 0;
    page->frame = (uint32_t) physical_address / PAGE_SIZE;

    //printf("trampoline at 0x%p (physical 0x%p)\r\n", trampoline_page, physical_address);

    // populate trampoline page with trampoline code
    for (int i = 0; i < sizeof(trampoline_data); i ++) {
        trampoline_page[i] = trampoline_data[i];
    }

    uint32_t *cr3_value = (uint32_t *) (trampoline_page + 1);
    uint32_t *entry_point = (uint32_t *) (trampoline_page + 20);

    *cr3_value = the_page_directory->physical_address;
    *entry_point = kernel_entry_point;

    /*printf("setting cr3 to 0x%p, entry point is 0x%p\r\n", *cr3_value, *entry_point);

    printf("trampoline data: ");
    for (int i = 0; i < sizeof(trampoline_data); i ++) {
        printf("0x%02x, ", trampoline_page[i]);
    }
    printf("\r\n");*/

    // bye-bye bootloader!
    printf("invoking trampoline...\r\n");
    ((void (*)(void)) trampoline_page)();
}
