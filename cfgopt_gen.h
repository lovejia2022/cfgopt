#ifndef CFGOPT_ls_H_
#define CFGOPT_ls_H_

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct cfgopt_ls {
	bool cfg_long;
	bool cfg_dereference;
	bool cfg_literal;
	int64_t cfg_block_size;
};

#ifndef CFGOPT_FATAL_EXIT
#define CFGOPT_FATAL_EXIT 1
#endif // CFGOPT_FATAL_EXIT

static inline void cfgopt_fatal(char const *message)
{
	fprintf(stderr, "cfgopt: %s\n", message);
	exit(CFGOPT_FATAL_EXIT);
}

void cfgopt_ls_init(struct cfgopt_ls *cfg)
#ifndef CFGOPT_IMPL_ls
;
#else
{
	cfg->cfg_long = false;
	cfg->cfg_dereference = false;
	cfg->cfg_literal = false;
	cfg->cfg_block_size = 0;
}
#endif // CFGOPT_IMPL_ls

#ifdef CFGOPT_IMPL_ls
void cfgopt_parse_string(char const *arg, char const **out) {}
void cfgopt_parse_int64(char const *arg, int64_t *out) {}
void cfgopt_parse_float64(char const *arg, double *out) {}
void cfgopt_parse_bool(char const *arg, bool *out) {}
#endif // CFGOPT_IMPL_ls

void cfgopt_ls_parse(struct cfgopt_ls *cfg, int argc, char **argv)
#ifndef CFGOPT_IMPL_ls
;
#else
{
	int argi;
	int len;

	for (argi = 0; argi < argc; ++argc) {

		len = strlen("long");
		if (strncmp("-long", argv[argi], len + 2)) {
			if (argv[argi][len] == 0) {
				// TODO: report error if no more argument
				argi += 1;
				cfgopt_parse_bool(
					argv[argi],
					&cfg->cfg_long
				);
				continue;
			} else if (argv[argi][len] == '=') {
				cfgopt_parse_bool(
					argv[argi] + len + 1,
					&cfg->cfg_long
				);
				continue;
			}
		}

		len = strlen("dereference");
		if (strncmp("-dereference", argv[argi], len + 2)) {
			if (argv[argi][len] == 0) {
				// TODO: report error if no more argument
				argi += 1;
				cfgopt_parse_bool(
					argv[argi],
					&cfg->cfg_dereference
				);
				continue;
			} else if (argv[argi][len] == '=') {
				cfgopt_parse_bool(
					argv[argi] + len + 1,
					&cfg->cfg_dereference
				);
				continue;
			}
		}

		len = strlen("literal");
		if (strncmp("-literal", argv[argi], len + 2)) {
			if (argv[argi][len] == 0) {
				// TODO: report error if no more argument
				argi += 1;
				cfgopt_parse_bool(
					argv[argi],
					&cfg->cfg_literal
				);
				continue;
			} else if (argv[argi][len] == '=') {
				cfgopt_parse_bool(
					argv[argi] + len + 1,
					&cfg->cfg_literal
				);
				continue;
			}
		}

		len = strlen("block_size");
		if (strncmp("-block_size", argv[argi], len + 2)) {
			if (argv[argi][len] == 0) {
				// TODO: report error if no more argument
				argi += 1;
				cfgopt_parse_int64(
					argv[argi],
					&cfg->cfg_block_size
				);
				continue;
			} else if (argv[argi][len] == '=') {
				cfgopt_parse_int64(
					argv[argi] + len + 1,
					&cfg->cfg_block_size
				);
				continue;
			}
		}

		// TODO: Report error for undefined flag.
		cfgopt_fatal("Undefined flag");
	}
}
#endif // CFGOPT_IMPL_ls

#endif // CFGOPT_ls_H_