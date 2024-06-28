#include <stdint.h>
#include "platform/openfirmware.h"

void __stdcall kmain(endpoint_t endpoint) {
    openfirmware_main(endpoint);
    while (1);
}
