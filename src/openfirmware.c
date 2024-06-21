#include "openfirmware.h"

/*
 * OpenFirmware interface based on kernie/impl/ppc/openfirmware.cpp from POBARISNA
 * (https://gitlab.com/sarahcrowle/pobarisna/-/blob/2d165e40adfd5f6acfdf9cc0867aee51634e265f/kernie/impl/ppc/openfirmware.cpp)
 */

void openfirmware_exit(int (*endpoint)(void *)) {
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

int32_t openfirmware_finddevice(int (*endpoint)(void *), const char *name) {
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

int32_t openfirmware_getprop(int (*endpoint)(void *), uint32_t handle, const char *property, void *buffer, uint32_t buffer_length) {
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

int32_t openfirmware_open(int (*endpoint)(void *), const char* device) {
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

uint32_t openfirmware_write(int (*endpoint)(void *), uint32_t handle, char *buffer, uint32_t buffer_length) {
    static struct {
        const char *cmd;
        int num_args;
        int num_returns;
        unsigned int handle;
        char* buffer;
        int buffer_length;
        int written_length;
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
