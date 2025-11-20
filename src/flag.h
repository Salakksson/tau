#ifndef FLAG_H
#define FLAG_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define FLAG_MAX 48

typedef enum {
    FLAG_INT,
    FLAG_BOOL,
    FLAG_CHAR_PTR,
} Flag_Type;

typedef struct {
    const char* description;
    const char* name;
    Flag_Type type;
    bool used;
} Flag_Information;

static Flag_Information _flag_main[FLAG_MAX];
static int _flag_count = 0;
static int _arg_global = 0;
static bool *_arg_used;

static void _track_args(int argc)
{
    if(argc != _arg_global) {
        if (_arg_used) free(_arg_used);
        _arg_used = calloc(argc, sizeof(bool));
        _arg_global = argc;
    }
}

static int _register_flag(const char *name, const char *description, Flag_Type type)
{
    if(_flag_count >= FLAG_MAX){
        printf("Capacity Is Greater than the max flag\n");
        return -1;
    }
    _flag_main[_flag_count].name        = name;
    _flag_main[_flag_count].description = description;
    _flag_main[_flag_count].type        = type;
    _flag_main[_flag_count].used        = false;
    return _flag_count++;
}

static bool _create_int(const char *str)
{
    if (!str || !*str) return false;
    char *endptr;
    strtol(str, &endptr, 10);
    return *endptr == '\0';
}

static bool _create_bool(const char *str)
{
    return (strcmp(str, "true") == 0 || strcmp(str, "false") == 0 ||
            strcmp(str, "1") == 0 || strcmp(str, "0") == 0);
}

static int _parse_int(int argc, char **argv, const char *name) {
    for (int i = 1; i < argc; i++) {
        if (_arg_used[i]) continue;

        if (argv[i][0] == '-') {
            char *eq = strchr(argv[i], '=');
            if (eq) {
                size_t name_len = eq - argv[i] - 1;
                if (strncmp(argv[i] + 1, name, name_len) == 0 && name[name_len] == '\0') {
                    char *value = eq + 1;
                    if (_create_int(value)) {
                        _arg_used[i] = true;
                        return atoi(value);
                    }
                }
            } else {
                if (strcmp(argv[i] + 1, name) == 0 && i + 1 < argc) {
                    if (!_arg_used[i + 1] && _create_int(argv[i + 1])) {
                        _arg_used[i] = true;
                        _arg_used[i + 1] = true;
                        return atoi(argv[i + 1]);
                    }
                }
            }
        }
    }

    for (int i = 1; i < argc; i++) {
        if (!_arg_used[i] && _create_int(argv[i])) {
            _arg_used[i] = true;
            return atoi(argv[i]);
        }
    }
    return 0;
}

static bool _parse_bool(int argc, char **argv, const char *name) {
    for (int i = 1; i < argc; i++) {
        if (_arg_used[i]) continue;

        if (argv[i][0] == '-') {
            char *eq = strchr(argv[i], '=');
            if (eq) {
                size_t name_len = eq - argv[i] - 1;
                if (strncmp(argv[i] + 1, name, name_len) == 0 && name[name_len] == '\0') {
                    char *value = eq + 1;
                    if (_create_bool(value)) {
                        _arg_used[i] = true;
                        return (strcmp(value, "true") == 0 || strcmp(value, "1") == 0);
                    }
                }
            } else {
                if (strcmp(argv[i] + 1, name) == 0 && i + 1 < argc) {
                    if (!_arg_used[i + 1] && _create_bool(argv[i + 1])) {
                        _arg_used[i] = true;
                        _arg_used[i + 1] = true;
                        return (strcmp(argv[i + 1], "true") == 0 || strcmp(argv[i + 1], "1") == 0);
                    }
                }
            }
        }
    }

    for (int i = 1; i < argc; i++) {
        if (!_arg_used[i] && _create_bool(argv[i])) {
            _arg_used[i] = true;
            return (strcmp(argv[i], "true") == 0 || strcmp(argv[i], "1") == 0);
        }
    }
    return false;
}

static char* _parse_char_ptr(int argc, char **argv, const char *name) {
    for (int i = 1; i < argc; i++) {
        if (_arg_used[i]) continue;

        if (argv[i][0] == '-') {
            char *eq = strchr(argv[i], '=');
            if (eq) {
                size_t name_len = eq - argv[i] - 1;
                if (strncmp(argv[i] + 1, name, name_len) == 0 && name[name_len] == '\0') {
                    char *value = eq + 1;
                    if (!_create_int(value) && !_create_bool(value)) {
                        _arg_used[i] = true;
                        return value;
                    }
                }
            } else {
                if (strcmp(argv[i] + 1, name) == 0 && i + 1 < argc) {
                    if (!_arg_used[i + 1] && !_create_int(argv[i + 1]) && !_create_bool(argv[i + 1])) {
                        _arg_used[i] = true;
                        _arg_used[i + 1] = true;
                        return argv[i + 1];
                    }
                }
            }
        }
    }

    for (int i = 1; i < argc; i++) {
        if (!_arg_used[i] && !_create_bool(argv[i]) && !_create_int(argv[i])) {
            _arg_used[i] = true;
            return argv[i];
        }
    }
    return NULL;
}

void help_flag(void) {
    printf("Available flags:\n");
    for (int i = 0; i < _flag_count; i++) {
        const char *type_str;
        switch (_flag_main[i].type) {
            case FLAG_INT: type_str = "(int)"; break;
            case FLAG_CHAR_PTR: type_str = "(char*)"; break;
            case FLAG_BOOL: type_str = "(bool)"; break;
            default: type_str = "(unknown)";
        }
        printf("-%s: %s \"%s\"\n", _flag_main[i].name, type_str, _flag_main[i].description);
    }
}

#define create_flag(argc, argv, type, name, description) \
    _Generic((type){0}, \
        int: ({ \
            _track_args(argc); \
            int _index = _register_flag(name, description, FLAG_INT); \
            _flag_main[_index].used = true; \
            _parse_int(argc, argv, name); \
        }), \
        bool: ({ \
            _track_args(argc); \
            int _index = _register_flag(name, description, FLAG_BOOL); \
            _flag_main[_index].used = true; \
            _parse_bool(argc, argv, name); \
        }), \
        char*: ({ \
            _track_args(argc); \
            int _index = _register_flag(name, description, FLAG_CHAR_PTR); \
            _flag_main[_index].used = true; \
            _parse_char_ptr(argc, argv, name); \
        }) \
    )

#endif //FLAG_H
