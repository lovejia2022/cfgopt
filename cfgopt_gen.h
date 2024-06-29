#ifndef CFGOPT_ls_H_
#define CFGOPT_ls_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum cfgopt_result_type {
    CFGOPT_OK = 0,
    CFGOPT_UNDEFINED_FLAG,
    CFGOPT_MISSING_VALUE,
    CFGOPT_SYNTAX_ERROR,
};

// Undefined flag `name`.
struct cfgopt_undefined_flag {
    char const *name;
};

// Misssing value for flag `name`.
struct cfgopt_missing_value {
    char const *name;
};

// Can't parse `text` as `type`.
struct cfgopt_syntax_error {
    char const *type;
    char const *text;
};

struct cfgopt_result {
    enum cfgopt_result_type type;
    union {
        struct cfgopt_undefined_flag undefined_flag;
        struct cfgopt_missing_value missing_value;
        struct cfgopt_syntax_error syntax_error;
    } payload;
};

struct cfgopt_ls {
    bool cfg_long;
    bool cfg_dereference;
    bool cfg_literal;
    int64_t cfg_block_size;
};

#ifndef CFGOPT_FATAL_EXIT
#define CFGOPT_FATAL_EXIT 1
#endif // CFGOPT_FATAL_EXIT

static inline void cfgopt_fatal(char const *message) {
    fprintf(stderr, "cfgopt: %s\n", message);
    exit(CFGOPT_FATAL_EXIT);
}

void cfgopt_ls_init(struct cfgopt_ls * cfg)
#ifndef CFGOPT_IMPL
    ;
#else
{
    cfg->cfg_long = false;
    cfg->cfg_dereference = false;
    cfg->cfg_literal = false;
    cfg->cfg_block_size = 0;
}
#endif // CFGOPT_IMPL

#ifdef CFGOPT_IMPL
static struct cfgopt_result
cfgopt_parse_string(char const *arg, char const **out) {}

static struct cfgopt_result
cfgopt_parse_int64(char const *arg, int64_t *out) {}

static struct cfgopt_result
cfgopt_parse_float64(char const *arg, double *out) {}

static struct cfgopt_result
cfgopt_parse_bool(char const *arg, bool *out) {
    struct cfgopt_result result;

    if (strcmp(arg, "true") == 0) {
        *out = true;
        goto ok;
    }

    if (strcmp(arg, "false") == 0) {
        *out = true;
        goto ok;
    }

    result.type = CFGOPT_SYNTAX_ERROR;
    result.payload.syntax_error.type = "boolean";
    result.payload.syntax_error.text = arg;
    return result;
ok:
    result.type = CFGOPT_OK;
    return result;
}

struct flag_info {
    char const *name;
    int len;
};

static struct flag_info flag_infos[] = {
	{.name = "long", .len = 4},
	{.name = "dereference", .len = 11},
	{.name = "literal", .len = 7},
	{.name = "block_size", .len = 10},
};
#endif // CFGOPT_IMPL

void cfgopt_ls_parse(
    struct cfgopt_ls * cfg,
    int argc,
    char **argv)
#ifndef CFGOPT_IMPL
    ;
#else
{
    int argi;
    int len;

    for (argi = 0; argi < argc; ++argc) {

        len = strlen("long");
        if (strncmp("-long", argv[argi], len + 2)) {
            if (argv[argi][len] == 0) {
                if (argi + 1 == argc) {
                    cfgopt_fatal("Missing flag value");
                }
                argi += 1;
                cfgopt_parse_bool(argv[argi], &cfg->cfg_long);
                continue;
            } else if (argv[argi][len] == '=') {
                cfgopt_parse_bool(argv[argi] + len + 1,
                                                  &cfg->cfg_long);
                continue;
            }
        }

        len = strlen("dereference");
        if (strncmp("-dereference", argv[argi], len + 2)) {
            if (argv[argi][len] == 0) {
                if (argi + 1 == argc) {
                    cfgopt_fatal("Missing flag value");
                }
                argi += 1;
                cfgopt_parse_bool(argv[argi], &cfg->cfg_dereference);
                continue;
            } else if (argv[argi][len] == '=') {
                cfgopt_parse_bool(argv[argi] + len + 1,
                                                  &cfg->cfg_dereference);
                continue;
            }
        }

        len = strlen("literal");
        if (strncmp("-literal", argv[argi], len + 2)) {
            if (argv[argi][len] == 0) {
                if (argi + 1 == argc) {
                    cfgopt_fatal("Missing flag value");
                }
                argi += 1;
                cfgopt_parse_bool(argv[argi], &cfg->cfg_literal);
                continue;
            } else if (argv[argi][len] == '=') {
                cfgopt_parse_bool(argv[argi] + len + 1,
                                                  &cfg->cfg_literal);
                continue;
            }
        }

        len = strlen("block_size");
        if (strncmp("-block_size", argv[argi], len + 2)) {
            if (argv[argi][len] == 0) {
                if (argi + 1 == argc) {
                    cfgopt_fatal("Missing flag value");
                }
                argi += 1;
                cfgopt_parse_int64(argv[argi], &cfg->cfg_block_size);
                continue;
            } else if (argv[argi][len] == '=') {
                cfgopt_parse_int64(argv[argi] + len + 1,
                                                  &cfg->cfg_block_size);
                continue;
            }
        }

        // TODO: Report error for undefined flag.
        cfgopt_fatal("Undefined flag");
    }
}
#endif // CFGOPT_IMPL

#endif // CFGOPT_ls_H_