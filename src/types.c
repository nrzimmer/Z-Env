#include "types.h"
#include "nob.h"

String_Builder sb_from_string_list(const StringList *da) {
    String_Builder sb = {0};
    sb_append(&sb, '/');
    for (size_t i = 0; i < da->count; ++i) {
        sb_append_cstr(&sb, da->items[i]);
        sb_append(&sb, '/');
    }
    return sb;
}