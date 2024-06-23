#ifdef OPENFIRMWARE

#include "openfirmware.h"
#include <stddef.h>

/*
 * OpenFirmware interface based on kernie/impl/ppc/openfirmware.cpp from POBARISNA
 * (https://gitlab.com/sarahcrowle/pobarisna/-/blob/2d165e40adfd5f6acfdf9cc0867aee51634e265f/kernie/impl/ppc/openfirmware.cpp)
 */

void openfirmware_exit(endpoint_t endpoint) {
    static struct {
        const char *cmd;
        int num_args;
        int num_returns;
    } args = {
        "exit",
        0,
        0
    };

    endpoint(&args);
}

int32_t openfirmware_finddevice(endpoint_t endpoint, const char *name) {
    static struct {
        const char *cmd;
        int num_args;
        int num_returns;
        const char *device;
        int handle;
    } args = {
        "finddevice",
        1,
        1
    };

    args.device = name;

    if (endpoint(&args) == -1) return -1;

    return args.handle;
}

int32_t openfirmware_getprop(endpoint_t endpoint, uint32_t handle, const char *property, void *buffer, uint32_t buffer_length) {
    static struct { 
        const char *cmd;
        int num_args;
        int num_returns;
        int handle;
        const char *property;
        void *buffer;
        int buffer_length;
        int written_size;
    } args = {
        "getprop",
        4,
        1
    };

    args.handle = handle;
    args.property = property;
    args.buffer = buffer;
    args.buffer_length = buffer_length;

    if (endpoint(&args) == -1) return -1;

    return args.written_size;
}

int32_t openfirmware_open(endpoint_t endpoint, const char* device) {
    static struct {
        const char *cmd;
        int num_args;
        int num_returns;
        const char *device_name;
        int handle;
    } args = {
        "open",
        1,
        1
    };

    args.device_name = device;
    if (endpoint(&args) == -1 || args.handle == 0) return -1;

    return args.handle;
}

uint32_t openfirmware_write(endpoint_t endpoint, int32_t handle, const char *buffer, uint32_t buffer_length) {
    static struct {
        const char *cmd;
        int num_args;
        int num_returns;
        int handle;
        const char *buffer;
        int buffer_length;
        unsigned int written_length;
    } args = {
        "write",
        3,
        1
    };

    args.handle = handle;
    args.buffer = buffer;
    args.buffer_length = buffer_length;
    args.written_length = -1;

    endpoint(&args);
    return args.written_length;
}

int32_t big_endian_to_native_endian(int32_t value) {
    uint8_t *p = (uint8_t *) &value;
    int32_t res = 0;

    for (int i = 0; i < sizeof(int32_t); i++)
        res = (res << 8) + p[i];
    return res;
}

int32_t openfirmware_find_stdout(endpoint_t endpoint) {
    // stdout handle acquiring code based on kernie/impl/ppc/kernie_special_ppc.cpp from POBARISNA
    // (https://gitlab.com/sarahcrowle/pobarisna/-/blob/2d165e40adfd5f6acfdf9cc0867aee51634e265f/kernie/impl/ppc/kernie_special_ppc.cpp)

    int32_t stdout_id;

    int32_t chosen = openfirmware_finddevice(endpoint, "/chosen"); // "chosen" doesn't work on OpenBIOS, the leading / is required
    if (chosen == -1) goto try_screen;

    int32_t size_written = openfirmware_getprop(endpoint, chosen, "stdout", &stdout_id, sizeof(stdout_id));
    if (size_written != sizeof(stdout_id)) goto try_screen;

    stdout_id = big_endian_to_native_endian(stdout_id);

    if (stdout_id == 0) {
try_screen:
        stdout_id = openfirmware_open(endpoint, "/screen");
    }

    if (stdout_id == 0) {
        return -1;
    } else {
        return stdout_id;
    }
}

struct putchar_state {
    endpoint_t endpoint;
    int32_t handle;
} putchar_state = {
    NULL,
    -1
};

void openfirmware_setup_putchar(endpoint_t endpoint) {
    putchar_state.endpoint = endpoint;
    putchar_state.handle = openfirmware_find_stdout(endpoint);
}

void _putchar(char c) {
    if (putchar_state.handle == -1) return;
    openfirmware_write(putchar_state.endpoint, putchar_state.handle, &c, 1);
}

#endif
