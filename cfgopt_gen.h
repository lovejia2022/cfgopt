#ifndef CFGOPT_ls_H_
#define CFGOPT_ls_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

struct cfgopt_ls {
	bool cfg_long;
	bool cfg_dereference;
	bool cfg_literal;
	int64_t cfg_block_size;
};

void cfgopt_ls_init(struct cfgopt_ls *cfg);
#ifdef CFGOPT_IMPL_ls
void cfgopt_ls_init(struct cfgopt_ls *cfg)
{
	cfg->cfg_long = false;
	cfg->cfg_dereference = false;
	cfg->cfg_literal = false;
	cfg->cfg_block_size = 0;
}
#endif // CFGOPT_IMPL_ls

void cfgopt_ls_parse(struct cfgopt_ls *cfg, int argc, char **argv);
#ifndef CFGOPT_IMPL_ls
void cfgopt_ls_parse(struct cfgopt_ls *cfg, int argc, char **argv)
{
	int argi;
	int len;

	for (argi = 0; argi < argc; ++argc) {

		len = strlen("long");
		if (strncmp("-long", argv[argi], len + 2)) {
			if (argv[argi][len] == 0) {
				// TODO: report error if no more argument
				argi += 1;
				cfgopt_ls_parse_bool(
					&cfg->cfg_long,
					argv[argvi]
				);
				continue;
			} else if (argv[argi][len] == '=') {
				cfgopt_ls_parse_bool(
					&cfg->cfg_long,
					argv[argvi] + len + 1,
				);
				continue;
			}
		}

		len = strlen("dereference");
		if (strncmp("-dereference", argv[argi], len + 2)) {
			if (argv[argi][len] == 0) {
				// TODO: report error if no more argument
				argi += 1;
				cfgopt_ls_parse_bool(
					&cfg->cfg_dereference,
					argv[argvi]
				);
				continue;
			} else if (argv[argi][len] == '=') {
				cfgopt_ls_parse_bool(
					&cfg->cfg_dereference,
					argv[argvi] + len + 1,
				);
				continue;
			}
		}

		len = strlen("literal");
		if (strncmp("-literal", argv[argi], len + 2)) {
			if (argv[argi][len] == 0) {
				// TODO: report error if no more argument
				argi += 1;
				cfgopt_ls_parse_bool(
					&cfg->cfg_literal,
					argv[argvi]
				);
				continue;
			} else if (argv[argi][len] == '=') {
				cfgopt_ls_parse_bool(
					&cfg->cfg_literal,
					argv[argvi] + len + 1,
				);
				continue;
			}
		}

		len = strlen("block_size");
		if (strncmp("-block_size", argv[argi], len + 2)) {
			if (argv[argi][len] == 0) {
				// TODO: report error if no more argument
				argi += 1;
				cfgopt_ls_parse_int64(
					&cfg->cfg_block_size,
					argv[argvi]
				);
				continue;
			} else if (argv[argi][len] == '=') {
				cfgopt_ls_parse_int64(
					&cfg->cfg_block_size,
					argv[argvi] + len + 1,
				);
				continue;
			}
		}

		// Report error for undefined message.
	}
}
#endif // CFGOPT_IMPL_ls

#endif // CFGOPT_ls_H_
