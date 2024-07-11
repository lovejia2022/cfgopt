#include <assert.h>
#include <inttypes.h>
#include <stdint.h>

#define CFGOPT_IMPL
#include "cfgopt_gen.h"

int main(int argc, char const** argv) {
    struct cfgopt_args cfg;

    cfgopt_args_init(&cfg);

    struct cfgopt_result r = cfgopt_args_parse(&cfg, argc, argv);
    assert(r.type == CFGOPT_OK);

    printf("boolean: %d\n", cfg.cfg_boolean_flag);
    printf("int64: %" PRIi64 "\n", cfg.cfg_int64_flag);
    printf("float64: %f\n", cfg.cfg_float64_flag);
    printf("string: %s\n", cfg.cfg_string_flag);

    cfgopt_args_drop(&cfg);

    return 0;
}
