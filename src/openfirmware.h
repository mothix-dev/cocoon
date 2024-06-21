#pragma once
#include <stdint.h>

void openfirmware_exit(int (*endpoint)(void *));
int32_t openfirmware_finddevice(int (*endpoint)(void *), const char *name);
int32_t openfirmware_getprop(int (*endpoint)(void *), uint32_t handle, const char *property, void *buffer, uint32_t buffer_length);
int32_t openfirmware_open(int (*endpoint)(void *), const char* device);
uint32_t openfirmware_write(int (*endpoint)(void *), uint32_t handle, char *buffer, uint32_t buffer_length);
