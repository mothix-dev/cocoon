#pragma once
#include <stdint.h>

/* port I/O functions */

static inline uint8_t inb(uint16_t addr) {
    unsigned char result;

    __asm__ __volatile__ (
        "inb %1, %0"
        : "=a" (result)
        : "dN" (addr)
    );

    return result;
}

static inline void outb(uint16_t addr, uint8_t value) {
    __asm__ __volatile__ (
        "outb %1, %0"
        :
        : "dN" (addr), "a" (value)
    );
}
