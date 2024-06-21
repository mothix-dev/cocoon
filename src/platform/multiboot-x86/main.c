#include "io.h"

#define NANOPRINTF_IMPLEMENTATION
#include "nanoprintf.h"

#include "utils.h"

void kmain(void) {
    printf("HellOwOrld!\n");

    while (1);
}

void serial_putchar(int c, void *state) {
    // Wait for the serial port's fifo to not be empty
    while ((inb(0x3F8 + 5) & 0x20) == 0);
    // Send the byte out the serial port
    outb(0x3F8, c);

    // Also send to the bochs 0xe9 hack
    outb(0xe9, c);
}

int printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    const int result = npf_vpprintf(&serial_putchar, NULL, format, args);
    va_end(args);
    return result;
}
