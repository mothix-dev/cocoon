#pragma once

#include <stdint.h>
#include <stdbool.h>

// TODO: maybe change the calling convention based on architecture? not sure if any planned targets with openfirmware use a different calling convention
typedef int (__stdcall *endpoint_t)(void *);

void openfirmware_exit(endpoint_t endpoint);
int32_t openfirmware_finddevice(endpoint_t endpoint, const char *name);
int32_t openfirmware_getprop(endpoint_t endpoint, int32_t handle, const char *property, void *buffer, uint32_t buffer_length);
int32_t openfirmware_getproplen(endpoint_t endpoint, int32_t handle, const char *property);
const char *openfirmware_getpropstr(endpoint_t endpoint, int32_t handle, const char *property);
int32_t openfirmware_open(endpoint_t endpoint, const char* device);
uint32_t openfirmware_read(endpoint_t endpoint, int32_t handle, char *buffer, uint32_t buffer_length);
uint32_t openfirmware_write(endpoint_t endpoint, int32_t handle, const char *buffer, uint32_t buffer_length);
bool openfirmware_seek(endpoint_t endpoint, int32_t handle, int32_t pos_hi, int32_t pos_lo);
uint64_t openfirmware_translate(endpoint_t endpoint, int32_t mmu_handle, void *address);
int32_t openfirmware_find_stdout(endpoint_t endpoint);
void openfirmware_main(endpoint_t endpoint);
