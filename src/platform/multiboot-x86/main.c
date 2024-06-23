#include "io.h"
#include "printf.h"

void kmain(void) {
    printf("HellOwOrld!\r\n");

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
