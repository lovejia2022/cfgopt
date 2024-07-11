/* libcfgopt - support library for generated code of cfgopt.
 */

#include "cfgopt.h"

#include <stdbool.h> // bool
#include <stddef.h> // size_t
#include <stdint.h> // int64_t
#include <stdio.h> // fprintf, FILE
#include <stdlib.h> // strtoll, strtod
#include <string.h> // strcmp

#include "cfgopt_inner.h"

static struct cfgopt_result todo() { return (struct cfgopt_result){.type = CFGOPT_TODO}; }

void cfgopt_print_result(struct cfgopt_result* r, FILE* file) {
    switch (r->type) {
    case CFGOPT_OK:
        fprintf(file, "Ok");
        break;
    case CFGOPT_UNDEFINED_FLAG:
        fprintf(file, "Undefined flag: %s", r->u.name);
        break;
    case CFGOPT_MISSING_VALUE:
        fprintf(file,
                "Flag ``%s'' need a value, use ``-FLAG=VALUE'' or "
                "``-FLAG VALUE''",
                r->u.name);
        break;
    case CFGOPT_SYNTAX_ERROR:
        fprintf(file, "Can't parse ``%s'' as %s", r->u.syntax_error.text, r->u.syntax_error.type);
        break;
    case CFGOPT_TODO:
        fprintf(file, "Not implemented");
        break;
    }
}

static struct cfgopt_result parse_string(char const* arg, char const** out) {
    struct cfgopt_result r;
    *out = arg;
    r.type = CFGOPT_OK;
    return r;
}

static struct cfgopt_result parse_int64(char const* arg, int64_t* out) {
    struct cfgopt_result r;
    *out = strtoll(arg, NULL, 10);
    r.type = CFGOPT_OK;
    return r;
}

static struct cfgopt_result parse_float64(char const* arg, double* out) {
    struct cfgopt_result r;
    *out = strtod(arg, NULL);
    r.type = CFGOPT_OK;
    return r;
}

static struct cfgopt_result parse_boolean(char const* arg, bool* out) {
    struct cfgopt_result r;

    if (strcmp(arg, "true") == 0) {
        *out = true;
        goto ok;
    }

    if (strcmp(arg, "false") == 0) {
        *out = false;
        goto ok;
    }

    r.type = CFGOPT_SYNTAX_ERROR;
    r.u.syntax_error.type = "boolean";
    r.u.syntax_error.text = arg;
    return r;
ok:
    r.type = CFGOPT_OK;
    return r;
}

static struct cfgopt_result
parse_flag_value(enum cfgopt_flag_type flag_type, char const* flag_value, void* flag_value_out) {
    bool boolean_tmp;
    struct cfgopt_result r;

    switch (flag_type) {
    case CFGOPT_BOOLEAN:
        return parse_boolean(flag_value, (bool*)flag_value_out);
    case CFGOPT_INT64:
        return parse_int64(flag_value, (int64_t*)flag_value_out);
    case CFGOPT_FLOAT64:
        return parse_float64(flag_value, (double*)flag_value_out);
    case CFGOPT_STRING:
        return parse_string(flag_value, (char const**)flag_value_out);
    case CFGOPT_BOOLEAN_ARRAY:
        r = parse_boolean(flag_value, &boolean_tmp);

        if (r.type == CFGOPT_OK) {
            cfgopt_append(*(struct cfgopt_boolean_array*)flag_value_out, boolean_tmp);
        }
        return r;
    default:
        return todo();
    }
}

struct cfgopt_parser cfgopt_new_parser(char const** args, size_t argc) {
    struct cfgopt_parser iter;
    iter.args = args;
    iter.argc = argc;
    iter.curr_arg = 1;
    iter.curr_pos = 0;
    iter.flag_value = NULL;
    iter.cost_args = 0;
    return iter;
}

static char const* parser_current_arg(struct cfgopt_parser* p) {
    return p->args[p->curr_arg] + p->curr_pos;
}

static bool try_flag_on_arg(struct cfgopt_parser* p, struct cfgopt_flag_info const* flag) {
    char const* arg = parser_current_arg(p);
    size_t i;

    for (i = 0; flag->name[i] != '\0'; ++i) {
        if (flag->name[i] != arg[i]) {
            logging("flag name mismatch: %s vs %s", flag->name, arg);
            return false;
        }
    }

    if (arg[i] == '\0') {
        if (flag->type != CFGOPT_BOOLEAN) {
            if (p->curr_arg + 1 == p->argc)
                return false;

            p->flag_value = p->args[p->curr_arg + 1];
            p->cost_args = 2;
        } else {
            p->flag_value = "true";
            p->cost_args = 1;
        }
    } else if (arg[i] == '=') {
        /* NAME=VALUE */
        p->flag_value = arg + i + 1;
        p->cost_args = 1;
    } else {
        return false;
    }

    return true;
}

static struct cfgopt_result try_all_flags_on_arg(struct cfgopt_parser* p,
                                                 struct cfgopt_flag_info const* flags,
                                                 size_t flag_count) {
    size_t i;

    for (i = 0; i < flag_count; ++i) {
        logging("try flag %s", flags[i].name);

        if (!try_flag_on_arg(p, flags + i)) {
            continue;
        }

        p->matched_flag = flags + i;

        parse_flag_value(p->matched_flag->type, p->flag_value, p->matched_flag->value);

        p->curr_arg += p->cost_args;
        p->curr_pos = 0;
        return cfgopt_ok();
    }

    return cfgopt_new_undefined_flag(p->args[p->curr_arg] + p->curr_pos);
}

struct cfgopt_result
cfgopt_parse(struct cfgopt_parser* p, struct cfgopt_flag_info const* flags, size_t flag_count) {
    while (p->curr_arg < p->argc) {
        char const* arg = p->args[p->curr_arg];

        if (arg[0] == '-') {
            if (arg[1] == '-') {
                p->curr_pos += 2;

                try_all_flags_on_arg(p, flags, flag_count);
            } else {
                logging("TODO: short flag: %s", arg);
                return todo();
            }
        } else {
            logging("TODO: positional argument: %s", arg);
            return todo();
        }
    }

    return cfgopt_ok();
}
