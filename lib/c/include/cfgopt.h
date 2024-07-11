/* libcfgopt - support library for generated code of cfgopt.
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
#include <stddef.h> // size_t
#include <stdint.h> // int64_t
#include <stdio.h> // FILE

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
    char const* type;
    char const* text;
};

/* Result infomation for functions. */
struct cfgopt_result {
    enum cfgopt_result_type type;
    union {
        char const* name;
        struct cfgopt_syntax_error syntax_error;
    } u;
};

/* Create a result for success. */
static inline struct cfgopt_result cfgopt_ok() { return (struct cfgopt_result){.type = CFGOPT_OK}; }

/* Create a result for undefined flag error. */
static inline struct cfgopt_result cfgopt_new_undefined_flag(char const* name) {
    return (struct cfgopt_result){.type = CFGOPT_UNDEFINED_FLAG, .u.name = name};
}

/* Create a result for missing value error. This occurs when find a
 * non-boolean flag in the command line arguments and can't find a value to
 * that flag.
 */
static inline struct cfgopt_result cfgopt_new_missing_value_error(char const* name) {
    return (struct cfgopt_result){.type = CFGOPT_MISSING_VALUE, .u.name = name};
}

/* Create a result for syntax error. This means argument string of a flag value
 * can't be parsed as the type of the flag.
 */
static inline struct cfgopt_result cfgopt_new_syntax_error(char const* type, char const* text) {
    return (struct cfgopt_result){
        .type = CFGOPT_SYNTAX_ERROR,
        .u.syntax_error.type = type,
        .u.syntax_error.text = text,
    };
}

/* Print the result to a file. The printed text looks like "Ok" or
 * "Can't parse 12.0 as boolean", without newline at the end.
 */
void cfgopt_print_result(struct cfgopt_result* r, FILE* file);

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
    int64_t* data;
    size_t len;
    size_t cap;
};

struct cfgopt_float64_array {
    double* data;
    size_t len;
    size_t cap;
};

struct cfgopt_boolean_array {
    bool* data;
    size_t len;
    size_t cap;
};

struct cfgopt_string_array {
    char const** data;
    size_t len;
    size_t cap;
};

/* Initialize list of array. */
#define cfgopt_array_init()                                                                        \
    { .data = NULL, .len = 0, .cap = 0 }

/* Drop array A, make sure A is initialized. */
#define cfgopt_array_drop(A) cfgopt_array_drop_((A))
#define cfgopt_array_drop_(A)                                                                      \
    do {                                                                                           \
        if (A->data != NULL) {                                                                     \
            free(A->data);                                                                         \
            A->data = NULL;                                                                        \
        }                                                                                          \
        A->len = 0;                                                                                \
        A->cap = 0;                                                                                \
    } while (0)

/* Append X to the rear of array A, make sure A is initialized. */
#define cfgopt_append(A, X) cfgopt_append_((A), (X))
#define cfgopt_append_(A, X)                                                                       \
    do {                                                                                           \
        if (A.len >= A.cap) {                                                                      \
            A.cap = A.cap == 0 ? 8 : A.cap * 2;                                                    \
            A.data = realloc(A.data, sizeof(*A.data) * A.cap);                                     \
        }                                                                                          \
        A.data[A.len] = X;                                                                         \
        A.len += 1;                                                                                \
    } while (0)

enum cfgopt_flag_type {
    CFGOPT_INT64,
    CFGOPT_BOOLEAN,
    CFGOPT_FLOAT64,
    CFGOPT_STRING,

    CFGOPT_INT64_ARRAY,
    CFGOPT_BOOLEAN_ARRAY,
    CFGOPT_FLOAT64_ARRAY,
    CFGOPT_STRING_ARRAY,
};

struct cfgopt_flag_info {
    char const* name;
    char short_name;

    enum cfgopt_flag_type type;
    void* value;

    char const* help;
};

struct cfgopt_parser {
    char const** args;
    size_t argc;
    size_t curr_arg;
    size_t curr_pos;

    /* Result of try_all_flags_on_arg. */

    char const* flag_value;
    int cost_args;
    struct cfgopt_flag_info const* matched_flag;
};

struct cfgopt_parser cfgopt_new_parser(char const** args, size_t argc);

struct cfgopt_result
cfgopt_parse(struct cfgopt_parser* p, struct cfgopt_flag_info const* flags, size_t flag_count);

#endif /* CFGOPT_H_ */
