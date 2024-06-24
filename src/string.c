#include "string.h"
#include "malloc.h"
#include "printf.h"

size_t strlen(const char *string) {
    size_t i = 0;
    for (; *string != 0; string ++, i ++);
    return i;
}

char *strdup(const char *string) {
    char *new_string = malloc(strlen(string) + 1);
    if (new_string == NULL) return NULL;

    for (char *c = new_string; (*c = *string) != 0; string ++, c ++);
    return new_string;
}

int strcmp(const char *a, const char *b) {
    for (;; a ++, b ++) {
        if (*a < *b)
            return -1;
        if (*a > *b)
            return 1;
        if (*a == 0)
            return 0;
    }
}

struct argument_pair *parse_arguments(const char *string) {
    char *argument_buffer = strdup(string);
    if (argument_buffer == NULL) return NULL;

    size_t num_pairs = 0;

    for (const char *c = argument_buffer; *c != 0; c ++)
        if (*c != ' ' && (c == argument_buffer || *(c - 1) == ' '))
            num_pairs ++;

    struct argument_pair *pairs = malloc((num_pairs + 1) * sizeof(struct argument_pair));
    if (pairs == NULL) return NULL;

    for (int i = 0; i <= num_pairs; i ++) {
        pairs[i].key = NULL;
        pairs[i].value = NULL;
    }

    if (num_pairs == 0) return pairs;

    const char *argument_start = NULL;
    struct argument_pair *current_pair = pairs;

    for (char *c = argument_buffer;; c ++) {
        char c_old = *c;

        if (*c != ' ' && argument_start == NULL) {
            argument_start = c;
        } else if (argument_start != NULL) {
            if (*c == '=') {
                current_pair->key = argument_start;
                argument_start = NULL;
                *c = 0;
            } else if (*c == ' ' || *c == 0) {
                if (current_pair->key == NULL)
                    current_pair->key = argument_start;
                else
                    current_pair->value = argument_start;
                argument_start = NULL;
                current_pair ++;
                *c = 0;
            }
        }

        if (c_old == 0) break;
    }

    return pairs;
}
