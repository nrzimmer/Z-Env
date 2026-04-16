#include "path.h"
#include "nob.h"

StringList *get_path_parts(const char *pwd) {
    String_View sv = sv_from_cstr(pwd);
    sv_chop_prefix(&sv, sv_from_cstr("/"));
    sv_chop_suffix(&sv, sv_from_cstr("/"));

    StringList *parts = calloc(1, sizeof(StringList));
    String_View part = sv_chop_by_delim(&sv, '/');
    while (part.count != 0) {
        da_append(parts, strndup(part.data, part.count));
        part = sv_chop_by_delim(&sv, '/');
    }
    return parts;
}

static char *normalize_path(const char *path, bool is_file) {
    String_View sv = sv_from_cstr(path);
    StringList segments = {0};

    String_View token = sv_chop_by_delim(&sv, '/');
    while (token.data[0] != 0) {
        if (token.count > 0) {
            if (token.count == 1 && strncmp(token.data, ".", 1) == 0) {
                // skip
            } else if (token.count == 2 && strncmp(token.data, "..", 2) == 0) {
                if (segments.count > 0) {
                    --segments.count;
                }
            } else {
                da_append(&segments, strdup(temp_sv_to_cstr(token)));
            }
        }
        token = sv_chop_by_delim(&sv, '/');
    }

    String_Builder sb = {0};
    sb_append(&sb, '/');

    for (size_t i = 0; i < segments.count; ++i) {
        sb_append_cstr(&sb, segments.items[i]);
        sb_append_cstr(&sb, "/");
    }
    if (is_file) {
        --sb.count;
    }
    sb_append_nullptr(&sb);


    return strdup(sb.items);
}

static char *expand_path_inner(const char *path, bool is_file) {
    NOB_ASSERT(path != nullptr && "Cannot expand if path is null.");

    String_Builder sb = {0};

    if (path[0] == '~') {
        const char *home = getenv("HOME");
        NOB_ASSERT(home != nullptr && "Could not expand ~. HOME is not set.");

        sb_append_cstr(&sb, home);
        sb_append_cstr(&sb, &(path[1]));
    } else {
        sb_append_cstr(&sb, path);
    }

    char *temp = strndup(sb.items, sb.count);

    sb.count = 0;
    if (temp[0] == '/') {
        sb_append_cstr(&sb, temp);
    } else {
        char *pwd = getcwd(nullptr, 0);
        NOB_ASSERT(pwd != nullptr && "Could not get current working directory.");

        sb_append_cstr(&sb, pwd);
        sb_append_cstr(&sb, "/");
        sb_append_cstr(&sb, temp);
    }
    sb_append_nullptr(&sb);

    return normalize_path(sb.items, is_file);
}

char *expand_path(const char *path) {
    return expand_path_inner(path, false);
}

char *expand_path_file(const char *path) {
    return expand_path_inner(path, true);
}

int is_directory(const char *path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return 0;
    }
    return S_ISDIR(st.st_mode);
}