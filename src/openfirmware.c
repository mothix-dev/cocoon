#ifdef OPENFIRMWARE

#include "openfirmware.h"
#include <stddef.h>
#include "printf.h"
#include "version.h"
#include "malloc.h"
#include "string.h"

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

int32_t openfirmware_getprop(endpoint_t endpoint, int32_t handle, const char *property, void *buffer, uint32_t buffer_length) {
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

int32_t openfirmware_getproplen(endpoint_t endpoint, int32_t handle, const char *property) {
    static struct {
        const char *cmd;
        int num_args;
        int num_returns;
        int handle;
        const char *property;
        int length;
    } args = {
        "getproplen",
        2,
        1
    };

    args.handle = handle;
    args.property = property;

    if (endpoint(&args) == -1) return -1;

    return args.length;
}

const char *openfirmware_getpropstr(endpoint_t endpoint, int32_t handle, const char *property) {
    int32_t length = openfirmware_getproplen(endpoint, handle, property);
    if (length == 0 || length == -1) return NULL;

    char *buffer = malloc(length);
    int32_t result = openfirmware_getprop(endpoint, handle, property, (void *) buffer, length);

    if (result != length) {
        free(buffer);
        return NULL;
    }

    return (const char *) buffer;
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

uint32_t openfirmware_read(endpoint_t endpoint, int32_t handle, char *buffer, uint32_t buffer_length) {
    static struct {
        const char *cmd;
        int num_args;
        int num_returns;
        int handle;
        const char *buffer;
        int buffer_length;
        unsigned int read_length;
    } args = {
        "read",
        3,
        1
    };

    args.handle = handle;
    args.buffer = buffer;
    args.buffer_length = buffer_length;
    args.read_length = -1;

    endpoint(&args);
    return args.read_length;
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

struct putchar_state {
    endpoint_t endpoint;
    int32_t handle;
} putchar_state = {
    NULL,
    -1
};

void *claim(void *address, size_t size, size_t alignment) {
    if (putchar_state.endpoint == NULL) return NULL;

    static struct {
        const char *cmd;
        int num_args;
        int num_returns;
        void *address;
        size_t size;
        size_t alignment;
        void *actual_address;
    } args = {
        "claim",
        3,
        1
    };

    args.address = address;
    args.size = size;
    args.alignment = alignment;

    if (putchar_state.endpoint(&args) == -1 || (size_t) args.actual_address == -1) return NULL;

    return args.actual_address;
}

static const char *resolve_relative_path(const char *absolute, const char *relative) {
    // calculate the worst case path buffer size
    size_t path_buffer_size = strlen(absolute) + strlen(relative) + 1;

    char *path_buffer = malloc(path_buffer_size);
    if (path_buffer == NULL) {
        printf("FATAL: couldn't allocate memory for path buffer\r\n");
        while (1);
    }

    int i = 0;

    // add node address from absolute path
    for (const char *c = absolute; *c != 0 && *c != ':' && i < path_buffer_size - 1; c ++) {
        path_buffer[i ++] = *c;
    }

    // add node address/arguments separator
    path_buffer[i ++] = ':';

    // add partition specifier
    int index_after_separator = i;
    for (const char *c = &absolute[i]; *c != 0 && ((*c >= '0' && *c <= '9') || *c == ',') && i < path_buffer_size - 1; c ++) {
        path_buffer[i ++] = *c;
    }
    // if the last character found wasn't a partition separator, undo this operation
    if (path_buffer[i - 1] != ',') {
        i = index_after_separator;
    }

    // if the initrd path is relative, fill in the path before it up until the filename of the kernel
    if (*relative != '/') {
        int last_slash = strlen(absolute);
        for (; last_slash > i && absolute[last_slash] != '\\'; last_slash --);
        for (; i <= last_slash && i < path_buffer_size - 1; i ++) {
            path_buffer[i] = absolute[i];
        }
    }

    // add the initrd path
    for (const char *c = relative; *c != 0 && i < path_buffer_size - 1; c ++) {
        path_buffer[i ++] = *c == '/' ? '\\' : *c;
    }

    // add null terminator
    path_buffer[i] = path_buffer[path_buffer_size - 1] = 0;

    return path_buffer;
}

void openfirmware_main(endpoint_t endpoint) {
    // stdout handle acquiring code based on kernie/impl/ppc/kernie_special_ppc.cpp from POBARISNA
    // (https://gitlab.com/sarahcrowle/pobarisna/-/blob/2d165e40adfd5f6acfdf9cc0867aee51634e265f/kernie/impl/ppc/kernie_special_ppc.cpp)

    int32_t stdout_id;

    int32_t chosen_id = openfirmware_finddevice(endpoint, "/chosen"); // "chosen" doesn't work on OpenBIOS, the leading / is required
    if (chosen_id == -1) goto try_screen;

    int32_t size_written = openfirmware_getprop(endpoint, chosen_id, "stdout", &stdout_id, sizeof(stdout_id));
    if (size_written != sizeof(stdout_id)) goto try_screen;

    stdout_id = big_endian_to_native_endian(stdout_id);

    if (stdout_id == 0) {
try_screen:
        stdout_id = openfirmware_open(endpoint, "/screen");
    }

    putchar_state.endpoint = endpoint;
    putchar_state.handle = stdout_id;

    printf(NAME_VERSION_INFO "\r\n");

    init_heap();

    const char *bootpath = openfirmware_getpropstr(endpoint, chosen_id, "bootpath");
    const char *cmdline = openfirmware_getpropstr(endpoint, chosen_id, "bootargs");
    printf("path: \"%s\", command line: \"%s\"\r\n", bootpath, cmdline);

    struct argument_pair *pairs = parse_arguments(cmdline);

    const char *initrd_filename = "initrd.tar";
    for (struct argument_pair *p = pairs; p->key != NULL; p ++)
        if (strcmp(p->key, "initrd") == 0 && p->value != NULL) {
            initrd_filename = p->value;
            break;
        }

    printf("using \"%s\" as initrd\r\n", initrd_filename);

    const char *absolute_initrd_path = resolve_relative_path(bootpath, initrd_filename);
    printf("resolved initrd path to \"%s\"\r\n", absolute_initrd_path);

    int32_t handle = openfirmware_open(endpoint, absolute_initrd_path);
    if (handle == 0 || handle == -1) {
        printf("FATAL: couldn't open initrd\r\n");
        while (1);
    }

    printf("initrd contents: \"");
    for (char c; openfirmware_read(endpoint, handle, &c, 1) == 1;) {
        if (c == '\r' || c == '\n')
            printf("\r\n");
        else
            printf("%c", c);
    }
    printf("\"\r\n");

    print_memory_blocks();
}

void _putchar(char c) {
    if (putchar_state.handle == 0 || putchar_state.handle == -1) return;
    openfirmware_write(putchar_state.endpoint, putchar_state.handle, &c, 1);
}

#endif
