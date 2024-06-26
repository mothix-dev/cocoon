/*
 * Multiboot initialization code, based on arch/x86/start.S from John Hodge's Rust Bare-bones Kernel
 * (https://github.com/thepowersgang/rust-barebones-kernel/blob/5523d62bea8cdbeafc1753e3bdefeb91d2a8a4b8/Kernel/arch/x86/start.S)
 */

MULTIBOOT_PAGE_ALIGN = (1<<0)
MULTIBOOT_MEMORY_INFO = (1<<1)
MULTIBOOT_REQVIDMODE = (1<<2)

MULTIBOOT_HEADER_MAGIC = 0x1badb002
MULTIBOOT_HEADER_FLAGS = MULTIBOOT_MEMORY_INFO
MULTIBOOT_CHECKSUM = -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

.section .multiboot, "a"
.globl header

header:
    .long MULTIBOOT_HEADER_MAGIC
    .long MULTIBOOT_HEADER_FLAGS
    .long MULTIBOOT_CHECKSUM
    .long header
    /* a.out kludge (not used, the kernel is elf) */
    .long 0, 0, 0, 0 /* load_addr, load_end_addr, bss_end_addr, entry_addr */
    /* Video mode */
    .long 0 /* Mode type (0: LFB) */
    .long 0 /* Width (no preference) */
    .long 0 /* Height (no preference) */
    .long 32 /* Depth (32-bit preferred) */

.section .inittext, "ax"
.globl start

start:
    mov %ebx, %edx /* put the multiboot structure pointer in the right register for the 2nd argument with regparm stdcall calling convention */

    lgdt gdt_ptr
    ljmp $0x08, $start_high

.section .text
.globl start_high
.extern kmain

start_high:
    /* Prep segment registers */
    mov $0x10, %bx
    mov %bx, %ss
    mov %bx, %ds
    mov %bx, %es
    mov %bx, %fs
    mov %bx, %gs

    mov $stack_end, %esp
    call kmain

    /* If kmain returns, halt forever */
halt:
    cli
    hlt
    jmp halt

.section .data
gdt_ptr:
    .word gdt_end - gdt - 1
    .long gdt
gdt:
    .long 0x00000000, 0x00000000 /* 00 NULL Entry */
    .long 0x0000FFFF, 0x00CF9A00 /* 08 PL0 Code */
    .long 0x0000FFFF, 0x00CF9200 /* 10 PL0 Data */
    .long 0x0000FFFF, 0x00CFFA00 /* 18 PL3 Code */
    .long 0x0000FFFF, 0x00CFF200 /* 20 PL3 Data */
gdt_end:

.section .bss
.globl stack_base
.globl stack_end

stack_base:
    .space 0x1000 * 3
stack_end:
