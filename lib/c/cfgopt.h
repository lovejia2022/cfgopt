/*
libcfgopt - support library for generated code of cfgopt.

This is a header only library, get implemention define CFGOPT_IMPL before
include this file.
*/

#ifndef CFGOPT_H_
#define CFGOPT_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

enum cfgopt_result_type {
	CFGOPT_OK = 0,
	CFGOPT_TODO,
	CFGOPT_UNDEFINED_FLAG,
	CFGOPT_MISSING_VALUE,
	CFGOPT_SYNTAX_ERROR,
};

/* Can't parse `text` as `type`. */
struct cfgopt_syntax_error {
	char const *type;
	char const *text;
};

struct cfgopt_result {
	enum cfgopt_result_type type;
	union {
		char const *name;
		struct cfgopt_syntax_error syntax_error;
	} u;
};

static inline struct cfgopt_result cfgopt_ok()
{
	return (struct cfgopt_result){.type = CFGOPT_OK};
}

static inline struct cfgopt_result cfgopt_new_undefined_flag(char const *name)
{
	return (struct cfgopt_result){.type = CFGOPT_UNDEFINED_FLAG,
				      .u.name = name};
}

static inline struct cfgopt_result
cfgopt_new_missing_value_error(char const *name)
{
	return (struct cfgopt_result){.type = CFGOPT_MISSING_VALUE,
				      .u.name = name};
}

static inline struct cfgopt_result cfgopt_new_syntax_error(char const *type,
							   char const *text)
{
	return (struct cfgopt_result){
		.type = CFGOPT_SYNTAX_ERROR,
		.u.syntax_error.type = type,
		.u.syntax_error.text = text,
	};
}

void cfgopt_print_result(struct cfgopt_result *r, FILE *file);

#ifndef CFGOPT_FATAL_EXIT
#define CFGOPT_FATAL_EXIT 1
#endif // CFGOPT_FATAL_EXIT

static inline void cfgopt_fatal(char const *message)
{
	fprintf(stderr, "cfgopt: %s\n", message);
	exit(CFGOPT_FATAL_EXIT);
}

#endif /* CFGOPT_H_ */

#ifdef CFGOPT_IMPL

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef CFGOPT_CONFIG_LOG
#define logging(...)                                                           \
	do {                                                                   \
		fprintf(stderr, "# ");                                         \
		fprintf(stderr, __VA_ARGS__);                                  \
		fprintf(stderr, "\n");                                         \
		fflush(stderr);                                                \
	} while (0)
#else
#define logging(...)
#endif

void cfgopt_print_result(struct cfgopt_result *r, FILE *file)
{
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
		fprintf(file,
			"Can't parse ``%s'' as %s",
			r->u.syntax_error.text,
			r->u.syntax_error.type);
		break;
	case CFGOPT_TODO:
		fprintf(file, "Not implemented");
		break;
	}
}

static struct cfgopt_result parse_string(char const *arg, char const **out)
{
	struct cfgopt_result r;
	*out = arg;
	r.type = CFGOPT_OK;
	return r;
}

static struct cfgopt_result parse_int64(char const *arg, int64_t *out)
{
	struct cfgopt_result r;
	*out = strtoll(arg, NULL, 10);
	r.type = CFGOPT_OK;
	return r;
}

static struct cfgopt_result parse_float64(char const *arg, double *out)
{
	struct cfgopt_result r;
	*out = strtod(arg, NULL);
	r.type = CFGOPT_OK;
	return r;
}

static struct cfgopt_result parse_boolean(char const *arg, bool *out)
{
	struct cfgopt_result r;

	if (strcmp(arg, "true") == 0) {
		*out = true;
		goto ok;
	}

	if (strcmp(arg, "false") == 0) {
		*out = true;
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

enum flag_type {
	FLAG_INT64,
	FLAG_BOOLEAN,
	FLAG_FLOAT64,
	FLAG_STRING,
};

static struct cfgopt_result parse_flag_value(enum flag_type flag_type,
					     char const *flag_value,
					     void *flag_value_out)
{
	switch (flag_type) {
	case FLAG_BOOLEAN:
		return parse_boolean(flag_value, (bool *)flag_value_out);
	case FLAG_INT64:
		return parse_int64(flag_value, (int64_t *)flag_value_out);
	case FLAG_FLOAT64:
		return parse_float64(flag_value, (double *)flag_value_out);
	case FLAG_STRING:
		return parse_string(flag_value, (char const **)flag_value_out);
	}

	return cfgopt_ok();
}

struct flag_info {
	char const *name;
	// size_t len;
	char short_name;

	enum flag_type type;
	void *value;
};

struct parser {
	char const **args;
	size_t argc;
	size_t curr_arg;
	size_t curr_pos;

	/* Result of try_all_flags_on_args */

	char const *flag_value;
	int cost_args;
	struct flag_info const *matched_flag;
};

struct parser new_parser(char const **args, size_t argc)
{
	struct parser iter;
	iter.args = args;
	iter.argc = argc;
	iter.curr_arg = 1;
	iter.curr_pos = 0;
	iter.flag_value = NULL;
	iter.cost_args = 0;
	return iter;
}

static char const *parser_current_arg(struct parser *p)
{
	return p->args[p->curr_arg] + p->curr_pos;
}

static bool try_flag_on_arg(struct parser *p, struct flag_info const *flag)
{
	char const *arg = parser_current_arg(p);
	size_t i;

	for (i = 0; flag->name[i] != '\0'; ++i) {
		if (flag->name[i] != arg[i]) {
			logging("flag name mismatch: %s vs %s",
				flag->name,
				arg);
			return false;
		}
	}

	if (arg[i] == '\0') {
		if (flag->type != FLAG_BOOLEAN) {
			if (p->curr_arg + 1 == p->argc)
				return false;

			p->flag_value = p->args[p->curr_arg + 1];
			p->cost_args = 2;
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

static struct cfgopt_result try_all_flags_on_arg(struct parser *p,
						 struct flag_info const *flags,
						 size_t flag_count)
{
	size_t i;

	for (i = 0; i < flag_count; ++i) {
		logging("try flag %s", flags[i].name);

		if (!try_flag_on_arg(p, flags + i)) {
			continue;
		}

		p->matched_flag = flags + i;

		parse_flag_value(p->matched_flag->type,
				 p->flag_value,
				 p->matched_flag->value);

		p->curr_arg += p->cost_args;
		p->curr_pos = 0;
		return cfgopt_ok();
	}

	return cfgopt_new_undefined_flag(p->args[p->curr_arg] + p->curr_pos);
}

static struct cfgopt_result todo()
{
	return (struct cfgopt_result){.type = CFGOPT_TODO};
}

static struct cfgopt_result
parse(struct parser *p, struct flag_info const *flags, size_t flag_count)
{
	while (p->curr_arg < p->argc) {
		char const *arg = p->args[p->curr_arg];

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

#endif /* CFGOPT_IMPL */
