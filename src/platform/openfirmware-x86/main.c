#include <stdint.h>
#include "openfirmware.h"
#include "printf.h"

void __stdcall kmain(endpoint_t endpoint) {
    openfirmware_setup_putchar(endpoint);

    printf("HellOwOrld!\r\n");

    while (1);
}
