#include <stdint.h>
#include "openfirmware.h"

#define NANOPRINTF_IMPLEMENTATION
#include "nanoprintf.h"

#include "utils.h"

struct printchar_state {
    int (*endpoint)(void *);
    int32_t handle;
} printchar_state;

void kmain(uint32_t r3, uint32_t r4, int (*endpoint)(void *)) {
    // init code based on kernie/impl/ppc/kernie_special_ppc.cpp from POBARISNA
    // (https://gitlab.com/sarahcrowle/pobarisna/-/blob/2d165e40adfd5f6acfdf9cc0867aee51634e265f/kernie/impl/ppc/kernie_special_ppc.cpp)

    int32_t stdout_id;

    int32_t chosen = openfirmware_finddevice(endpoint, "/chosen"); // "chosen" doesn't work on OpenBIOS, the leading / is required
    if (chosen == -1) goto try_screen;

    int32_t size_written = openfirmware_getprop(endpoint, chosen, "stdout", &stdout_id, sizeof(stdout_id));
    if (size_written != sizeof(stdout_id)) goto try_screen;

    if (stdout_id == 0) {
try_screen:
        stdout_id = openfirmware_open(endpoint, "/screen");
        if (stdout_id == -1) openfirmware_exit(endpoint); // if no stdout can be found, just exit- there's no point trying
    }

    printchar_state.endpoint = endpoint;
    printchar_state.handle = stdout_id;

    printf("HellOwOrld!\n");

    while (1);
}

void openfirmware_putchar(int character, void *state) {
    char c = character;
    openfirmware_write(printchar_state.endpoint, printchar_state.handle, &c, 1);
}

int printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    const int result = npf_vpprintf(&openfirmware_putchar, NULL, format, args);
    va_end(args);
    return result;
}
