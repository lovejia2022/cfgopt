/* libcfgopt - support library for generated code of cfgopt.
 */

#ifndef CFGOPT_INNER_H_
#define CFGOPT_INNER_H_

#include "cfgopt.h"

#include <stdbool.h> // bool
#include <stddef.h> // size_t
#include <stdint.h> // int64_t
#include <stdio.h> // fprintf, FILE
#include <stdlib.h> // strtoll, strtod
#include <string.h> // strcmp

#ifdef CFGOPT_CONFIG_LOG
#define logging(...)                                                                               \
    do {                                                                                           \
        fprintf(stderr, "# ");                                                                     \
        fprintf(stderr, __VA_ARGS__);                                                              \
        fprintf(stderr, "\n");                                                                     \
        fflush(stderr);                                                                            \
    } while (0)
#else
#define logging(...)
#endif

#endif /* CFGOPT_INNER_H_ */
