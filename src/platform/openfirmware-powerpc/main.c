#include <stdint.h>
#include "openfirmware.h"
#include "printf.h"

void kmain(uint32_t r3, uint32_t r4, endpoint_t endpoint) {
    openfirmware_setup_putchar(endpoint);

    printf("HellOwOrld!\r\n");

    while (1);
}
