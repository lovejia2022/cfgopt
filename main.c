#include <inttypes.h>
#include <stdio.h>

#define CFGOPT_IMPL
#include "cfgopt_gen.h"

int main() {
  struct cfgopt_sample cfg;

  char const *args[] = {"foo", "-boolean_flag", "-int64_flag=1",
                        "-float64_flag=0.5", "-string_flag=foo"};
  struct cfgopt_result r =
      cfgopt_sample_parse(&cfg, sizeof(args) / sizeof(args[0]), args);

  if (r.type != CFGOPT_OK) {
    fprintf(stderr, "Error: ");
    cfgopt_print_result(&r, stderr);
    fprintf(stderr, "\n");
  }

  fprintf(stderr, "boolean: %d\n", cfg.cfg_boolean_flag);
  fprintf(stderr, "int64: %" PRIi64 "\n", cfg.cfg_int64_flag);
  fprintf(stderr, "float64: %g\n", cfg.cfg_float64_flag);
  fprintf(stderr, "string: %s\n", cfg.cfg_string_flag);
  return 0;
}
