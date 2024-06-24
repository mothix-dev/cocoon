#pragma once

#include <stddef.h>

size_t strlen(const char *string);
char *strdup(const char *string);
int strcmp(const char *a, const char *b);

struct argument_pair {
    const char *key;
    const char *value;
};

struct argument_pair *parse_arguments(const char *string);
