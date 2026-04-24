#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <string.h>

#include "dotenv.h"
#include "nob.h"

bool parse_dotenv(Variables *variables, char *filepath) {
    String_Builder sb = {0};
    if (!read_entire_file(filepath, &sb)) {
        return false;
    }
    String_View sv = sb_to_sv(sb);
    while (sv.count > 0) {
        String_View line = sv_chop_by_delim(&sv, '\n');
        line = sv_trim(line);

        if ((line.count == 0) ||
            (line.count > 0 && line.data[0] == '#') ||
            (line.count > 1 && line.data[0] == '/' && line.data[1] == '/'))
            continue;

        const String_View key = sv_chop_by_delim(&line, '=');
        if (line.count == 0) {
            nob_log(NOB_WARNING, "Missing value for key: %.*s", (int) key.count, key.data);
            continue;
        }

        const KeyValuePair *found = nullptr;
        for (size_t i = 0; i < variables->count; ++i) {
            if (sv_eq(variables->items[i].key, key)) {
                found = &variables->items[i];
                break;
            }
        }

        if (found) {
            nob_log(NOB_WARNING, "Duplicate key: %.*s with value: %.*s from %s\n\tUsing value: %.*s from %.*s",
                    (int) key.count, key.data, (int) line.count, line.data, filepath, (int) found->value.count,
                    found->value.data, (int) found->path.count, found->path.data);
            continue;
        }

        sv_chop_prefix(&line, sv_from_cstr("\""));
        sv_chop_suffix(&line, sv_from_cstr("\""));

        const KeyValuePair kv = {
            sv_from_cstr(strndup(key.data, key.count)),
            sv_from_cstr(strndup(line.data, line.count)),
            sv_from_cstr(filepath),
        };

        da_append(variables, kv);
    }
    free(sb.items);
    return true;
}

void free_variables(Variables *variables) {
    for (size_t i = 0; i < variables->count; i++) {
        free((char *)variables->items[i].key.data);
        free((char *)variables->items[i].value.data);
    }
    free(variables->items);
}
