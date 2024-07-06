/* libcfgopt - support library for generated code of cfgopt.
 *
 * This is a header only library, get implemention by define CFGOPT_IMPL before
 * include this file.
 *
 * If you define CFGOPT_CONFIG_LOG with CFGOPT_IMPL, libcfgopt will print logs
 * to show how arguments is parsed, just for development.
 *
 * cfgopt will generate a header file contains "struct cfgopt_args" and a list
 * of functions to use that struct. The generated header file will include this
 * library by name "cfgopt.h" to make generated code works. So in the workspace
 * you use the generated header, you should add this into the header serach path
 * list. For example add "-I<path/to/parent/of/cfgopt.h>" to command arguments
 * for gcc and clang.
 */

#ifndef CFGOPT_H_
#define CFGOPT_H_

#include <stdbool.h> // bool
#include <stddef.h>  // size_t
#include <stdint.h>  // int64_t
#include <stdio.h>   // FILE

/* Type of result type of functions, used in struct cfgopt_result. */
enum cfgopt_result_type {
	CFGOPT_OK = 0,
	CFGOPT_TODO,
	CFGOPT_UNDEFINED_FLAG,
	CFGOPT_MISSING_VALUE,
	CFGOPT_SYNTAX_ERROR,
};

/* Can't parse `text` as `type`. This is part of result type. */
struct cfgopt_syntax_error {
	char const *type;
	char const *text;
};

/* Result infomation for functions. */
struct cfgopt_result {
	enum cfgopt_result_type type;
	union {
		char const *name;
		struct cfgopt_syntax_error syntax_error;
	} u;
};

/* Create a result for success. */
static inline struct cfgopt_result cfgopt_ok()
{
	return (struct cfgopt_result){.type = CFGOPT_OK};
}

/* Create a result for undefined flag error. */
static inline struct cfgopt_result cfgopt_new_undefined_flag(char const *name)
{
	return (struct cfgopt_result){.type = CFGOPT_UNDEFINED_FLAG,
	                              .u.name = name};
}

/* Create a result for missing value error. This occurs when find a
 * non-boolean flag in the command line arguments and can't find a value to
 * that flag.
 */
static inline struct cfgopt_result
cfgopt_new_missing_value_error(char const *name)
{
	return (struct cfgopt_result){.type = CFGOPT_MISSING_VALUE,
	                              .u.name = name};
}

/* Create a result for syntax error. This means argument string of a flag value
 * can't be parsed as the type of the flag.
 */
static inline struct cfgopt_result cfgopt_new_syntax_error(char const *type,
                                                           char const *text)
{
	return (struct cfgopt_result){
	        .type = CFGOPT_SYNTAX_ERROR,
	        .u.syntax_error.type = type,
	        .u.syntax_error.text = text,
	};
}

/* Print the result to a file. The printed text looks like "Ok" or
 * "Can't parse 12.0 as boolean", without newline at the end.
 */
void cfgopt_print_result(struct cfgopt_result *r, FILE *file);

/* A list of array types for flags with "multiple = true", each array contains
 * pointer "data" to the starts of memory of values, length of current array
 * "len", and "cap" for the array capacity.
 *
 * Array should be init by assign it to "cfgopt_array_init()". User should call
 * "cfgopt_array_drop(&array)" to cleanup that array.
 *
 * If "array.data" is not NULL, user can use "array.data[i]" if
 * 0 <= i < array.len.
 *
 * Note that the generate function "cfgopt_arg_init" and "cfgopt_arg_drop" will
 * do those init and drop for you.
 */

struct cfgopt_int64_array {
	int64_t *data;
	size_t len;
	size_t cap;
};

struct cfgopt_float64_array {
	double *data;
	size_t len;
	size_t cap;
};

struct cfgopt_boolean_array {
	bool *data;
	size_t len;
	size_t cap;
};

struct cfgopt_string_array {
	char const **data;
	size_t len;
	size_t cap;
};

/* Initialize list of array. */
#define cfgopt_array_init()                                                    \
	{                                                                      \
		.data = NULL, .len = 0, .cap = 0                               \
	}

/* Drop array A, make sure A is initialized. */
#define cfgopt_array_drop(A) cfgopt_array_drop_((A))
#define cfgopt_array_drop_(A)                                                  \
	do {                                                                   \
		if (A->data != NULL) {                                         \
			free(A->data);                                         \
			A->data = NULL;                                        \
		}                                                              \
		A->len = 0;                                                    \
		A->cap = 0;                                                    \
	} while (0)

/* Append X to the rear of array A, make sure A is initialized. */
#define cfgopt_append(A, X) cfgopt_append_((A), (X))
#define cfgopt_append_(A, X)                                                   \
	do {                                                                   \
		if (A.len >= A.cap) {                                          \
			A.cap = A.cap == 0 ? 8 : A.cap * 2;                    \
			A.data = realloc(A.data, sizeof(*A.data) * A.cap);     \
		}                                                              \
		A.data[A.len] = X;                                             \
		A.len += 1;                                                    \
	} while (0)

#endif /* CFGOPT_H_ */

/* Starts of implemention of libcfgopt. User should not use symbols after. */

#ifdef CFGOPT_IMPL

#include <stdbool.h> // bool
#include <stddef.h>  // size_t
#include <stdint.h>  // int64_t
#include <stdio.h>   // fprintf, FILE
#include <stdlib.h>  // strtoll, strtod
#include <string.h>  // strcmp

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

static struct cfgopt_result todo()
{
	return (struct cfgopt_result){.type = CFGOPT_TODO};
}

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

enum flag_type {
	FLAG_INT64,
	FLAG_BOOLEAN,
	FLAG_FLOAT64,
	FLAG_STRING,

	FLAG_INT64_ARRAY,
	FLAG_BOOLEAN_ARRAY,
	FLAG_FLOAT64_ARRAY,
	FLAG_STRING_ARRAY,
};

static struct cfgopt_result parse_flag_value(enum flag_type flag_type,
                                             char const *flag_value,
                                             void *flag_value_out)
{
	bool boolean_tmp;
	struct cfgopt_result r;

	switch (flag_type) {
	case FLAG_BOOLEAN:
		return parse_boolean(flag_value, (bool *)flag_value_out);
	case FLAG_INT64:
		return parse_int64(flag_value, (int64_t *)flag_value_out);
	case FLAG_FLOAT64:
		return parse_float64(flag_value, (double *)flag_value_out);
	case FLAG_STRING:
		return parse_string(flag_value, (char const **)flag_value_out);
	case FLAG_BOOLEAN_ARRAY:
		r = parse_boolean(flag_value, &boolean_tmp);

		if (r.type == CFGOPT_OK) {
			cfgopt_append(
			        *(struct cfgopt_boolean_array *)flag_value_out,
			        boolean_tmp);
		}
		return r;
	default:
		return todo();
	}
}

struct flag_info {
	char const *name;
	char short_name;

	enum flag_type type;
	void *value;
};

struct parser {
	char const **args;
	size_t argc;
	size_t curr_arg;
	size_t curr_pos;

	/* Result of try_all_flags_on_arg. */

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
