#define CFGOPT_IMPL
#include "cfgopt_gen.h"

int main(int argc, char **argv) {
  struct cfgopt_ls cfg;

  cfgopt_ls_parse(&cfg, argc, argv);
  return 0;
}
