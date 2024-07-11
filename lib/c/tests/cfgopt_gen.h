/* Generated by cfgopt, DO NOT EDIT.
 * 
 * This is both header and source file. Define CFGOPT_IMPL before include
 * this header file makes this a source file.
 */

#ifndef CFGOPT_GEN_sample_H_
#define CFGOPT_GEN_sample_H_

#include <stdbool.h>
#include <stdint.h>

#include "cfgopt.h"

/* Command line arguments defined in a cfgopt.toml file. */
struct cfgopt_args {
    bool cfg_boolean_flag;
    struct cfgopt_boolean_array cfg_boolean_array;
    int64_t cfg_int64_flag;
    double cfg_float64_flag;
    char const * cfg_string_flag;
    bool cfg_help;
};

/* Initilize cfgopt_args. */
void cfgopt_args_init(struct cfgopt_args *cfg);

/* Print help of `cfg` to `fp` */
void cfgopt_args_print_help(FILE *fp);

/* Drop "cfg", make "cfg" is initialized. */
void cfgopt_args_drop(struct cfgopt_args *cfg);

/* Parse command line arguments, set result to "cfg". "argc" and "argv"
 * usually comes from arguments of the main function.
 */
struct cfgopt_result cfgopt_args_parse(struct cfgopt_args *cfg,
                                       int argc,
                                       char const **argv);

#endif // CFGOPT_GEN_sample_H_

/* End of header part and starts of implementions, define CFGOPT_IMPL to open
 * it.
 */

#ifdef CFGOPT_IMPL

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cfgopt_args_print_help(FILE *fp) {
    fprintf(
        fp,
        "Flags:\n"
        "  -b, --boolean_flag (boolean)  boolean flag sample\n"
        "  --boolean_array (boolean_array)  boolean array sample\n"
        "  -i, --int64_flag (int64) default: 1024, int64 flag sample\n"
        "  -f, --float64_flag (float64) default: 1024, int64 flag sample\n"
        "  -s, --string_flag (string)  string flag sample\n"
        "  -h, --help (boolean)  print this help and exit\n"
    );
}

void cfgopt_args_init(struct cfgopt_args *cfg) {
    cfg->cfg_boolean_flag = false;
    cfg->cfg_boolean_array = (struct cfgopt_boolean_array) cfgopt_array_init();
    cfg->cfg_int64_flag = 0;
    cfg->cfg_float64_flag = 0.0;
    cfg->cfg_string_flag = NULL;
    cfg->cfg_help = false;
}

void cfgopt_args_drop(struct cfgopt_args *cfg) {
    cfg->cfg_boolean_flag = false;
    cfgopt_array_drop(&cfg->cfg_boolean_array);
    cfg->cfg_int64_flag = 0;
    cfg->cfg_float64_flag = 0.0;
    cfg->cfg_string_flag = NULL;
    cfg->cfg_help = false;
}

#define cfgopt_get_flag_info(cfg) cfgopt_get_flag_info_((cfg))
#define cfgopt_get_flag_info_(cfg) \
{ \
    { \
        .name = "boolean_flag", \
        .type = CFGOPT_BOOLEAN, \
        .value = &cfg->cfg_boolean_flag, \
        .help = "boolean flag sample", \
    }, \
    { \
        .name = "boolean_array", \
        .type = CFGOPT_BOOLEAN_ARRAY, \
        .value = &cfg->cfg_boolean_array, \
        .help = "boolean array sample", \
    }, \
    { \
        .name = "int64_flag", \
        .type = CFGOPT_INT64, \
        .value = &cfg->cfg_int64_flag, \
        .help = "int64 flag sample", \
    }, \
    { \
        .name = "float64_flag", \
        .type = CFGOPT_FLOAT64, \
        .value = &cfg->cfg_float64_flag, \
        .help = "int64 flag sample", \
    }, \
    { \
        .name = "string_flag", \
        .type = CFGOPT_STRING, \
        .value = &cfg->cfg_string_flag, \
        .help = "string flag sample", \
    }, \
    { \
        .name = "help", \
        .type = CFGOPT_BOOLEAN, \
        .value = &cfg->cfg_help, \
        .help = "print this help and exit", \
    }, \
}

#ifndef CFGOPT_AUTO_HELP
#define CFGOPT_AUTO_HELP  1 
#endif

struct cfgopt_result cfgopt_args_parse(struct cfgopt_args *cfg,
                                       int argc,
                                       char const **argv) {
    struct cfgopt_flag_info flag_infos[] = cfgopt_get_flag_info(cfg);

    struct cfgopt_parser p = cfgopt_new_parser(argv, argc);
    struct cfgopt_result r = cfgopt_parse(&p, flag_infos, 6);

#if CFGOPT_AUTO_HELP
    if (cfg->cfg_help) {
        cfgopt_args_print_help(stdout);
        exit(0);
    }
#endif
    return r;
}

#endif // CFGOPT_IMPL