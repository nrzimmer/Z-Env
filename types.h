#ifndef ZENV_TYPES_H
#define ZENV_TYPES_H

#include <stddef.h>
#include "nob.h"

#define sv_dot_star(sv) (int)(sv).count, (sv).data

typedef struct {
    char **items;
    size_t count;
    size_t capacity;
} StringList;

String_Builder sb_from_string_list(const StringList *da);

#endif