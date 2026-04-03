// Need this because i am using c23
#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define NOB_IMPLEMENTATION
#include "nob.h"
#undef NOB_IMPLEMENTATION

#include "path.h"
#include "config.h"
#include "dotenv.h"
#include "cli.h"

static int run() {
    const char *pwd = getcwd(nullptr, 0);
    NOB_ASSERT(pwd != nullptr && "Could not get current working directory.");
    pwd = expand_path(pwd);

    StringList *parts = get_path_parts(pwd);
    Variables dot_env = {0};

    while (parts->count > 0) {
        String_Builder sb = sb_from_string_list(parts);
        if (is_path_allowed_sb(&sb)) {
            sb_append_cstr(&sb, "/.env");
            char *filepath = strndup(sb.items, sb.count);
            filepath = expand_path_file(filepath);
            if (!parse_dotenv(&dot_env, filepath)) {
                nob_log(NOB_ERROR, "Failed to parse .env file at %s", filepath);
                return 1;
            }
        }
        --parts->count;
    }

    for (size_t i = 0; i < dot_env.count; ++i) {
        printf("export %.*s=\"%.*s\"\n", sv_dot_star(dot_env.items[i].key), sv_dot_star(dot_env.items[i].value));
    }

    return 0;
}

int cd(char *old_path) {
    // First we need to traverse the old_path back until we find a common folder with the current path
    // If there is an allowed folder in each partial path, we need to unset the variables on that folder env

    StringList *old_parts = get_path_parts(old_path);
    const char *pwd = getcwd(nullptr, 0);
    NOB_ASSERT(pwd != nullptr && "Could not get current working directory.");
    pwd = expand_path(pwd);
    const StringList *parts = get_path_parts(pwd);

    const size_t max = old_parts->count > parts->count ? old_parts->count : parts->count;
    size_t same = 0;
    for (size_t i = 0; i < max; ++i) {
        if (strcmp(old_parts->items[i], parts->items[i]) == 0) {
            same = i;
        } else {
            break;
        }
    }

    for (size_t i = same; i < old_parts->count; ++i) {
        String_Builder sb = sb_from_string_list(old_parts);
        --old_parts->count;
        char *filepath = expand_path(strndup(sb.items, sb.count));
        if (is_path_allowed(filepath)) {
            sb_append_cstr(&sb, "/.env");
            filepath = expand_path(strndup(sb.items, sb.count));
            Variables dot_env = {0};
            if (parse_dotenv(&dot_env, filepath)) {
                for (size_t j = 0; j < dot_env.count; ++j) {
                    printf("unset %.*s\n", sv_dot_star(dot_env.items[j].key));
                }
            }
        }
    }

    // Now we run normally to set all the variables in current path
    // This may be unnecessary because it will be run before the next command too
    return run();
}

int hook(char * str) {
    const Shell shell = parse_shell(str);
    switch (shell) {
        case ZSH:
            return 0;
        case BASH:
        default:
            fprintf(stderr, "Unsupported shell: %s\n", str);
            return 1;
    }
}

int main(const int argc, const char **argv) {
    Params *params = parse_params(argc, argv);

    if (params->action == RUN)
        nob_minimal_log_level = NOB_ERROR;

    parse_config();

    if (params->text == nullptr && (params->action == ALLOW || params->action == DENY)) {
        const char *pwd = getcwd(nullptr, 0);
        NOB_ASSERT(pwd != nullptr && "Could not get current working directory.");
        params->text = expand_path(pwd);
    }

    switch (params->action) {
        case ALLOW:
            return allow_path(params->text);
        case DENY:
            return deny_path(params->text);
        case LIST:
            return list_paths();
        case RUN:
            return run();
        case CD:
            return cd(params->text);
        case HOOK:
            return hook(params->text);
        case HELP:
        default:


    }

    UNREACHABLE("This should not happen...");
}
