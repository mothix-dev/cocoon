#include <stdint.h>
#include "platform/openfirmware.h"

void kmain(uint32_t r3, uint32_t r4, endpoint_t endpoint) {
    openfirmware_main(endpoint);
    while (1);
}
