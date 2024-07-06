#include <assert.h>
#include <stdint.h>

#define CFGOPT_IMPL

#include "cfgopt_gen.h"
#include "cfgopt_inner.h"

int main()
{
	char const *foo = NULL;
	int64_t bar = 0;

	struct cfgopt_flag_info infos[] = {
	        {
	                .name = "foo",
	                .short_name = 'f',

	                .type = FLAG_STRING,
	                .value = &foo,
	        },
	        {
	                .name = "bar",
	                .short_name = 'b',

	                .type = FLAG_INT64,
	                .value = &bar,
	        },
	};

	char const *argv[] = {"prog", "--foo", "HELLO", "--bar=12"};

	struct cfgopt_parser p =
	        cfgopt_new_parser(argv, sizeof(argv) / sizeof(char const *));
	struct cfgopt_result r;

	r = cfgopt_parse(&p,
	                 infos,
	                 sizeof(infos) / sizeof(struct cfgopt_flag_info));

	assert(r.type == CFGOPT_OK);
	assert(strcmp(foo, "HELLO") == 0);
	assert(bar == 12);

	// Generated code test

	struct cfgopt_args cfg;

	cfgopt_args_init(&cfg);

	{
		logging("test: basic");

		char const *args[] = {"foo",
		                      "--boolean_flag",
		                      "--int64_flag=1",
		                      "--float64_flag=0.5",
		                      "--string_flag=foo"};

		r = cfgopt_args_parse(&cfg,
		                      sizeof(args) / sizeof(args[0]),
		                      args);
		assert(r.type == CFGOPT_OK);

		assert(cfg.cfg_boolean_flag);
		assert(cfg.cfg_int64_flag == 1);
		assert(cfg.cfg_float64_flag == 0.5);
		assert(strcmp(cfg.cfg_string_flag, "foo") == 0);
	}

	{
		logging("test: boolean=true");

		char const *args[] = {"foo", "--boolean_flag=true"};

		cfg.cfg_boolean_flag = false;
		r = cfgopt_args_parse(&cfg,
		                      sizeof(args) / sizeof(args[0]),
		                      args);
		assert(r.type == CFGOPT_OK);
		assert(cfg.cfg_boolean_flag);
	}

	{
		logging("test: boolean=false");

		char const *args[] = {"foo", "--boolean_flag=false"};

		cfg.cfg_boolean_flag = true;
		r = cfgopt_args_parse(&cfg,
		                      sizeof(args) / sizeof(args[0]),
		                      args);
		assert(r.type == CFGOPT_OK);
		assert(!cfg.cfg_boolean_flag);
	}

	{
		logging("test: boolean array");

		char const *args[] = {"foo",
		                      "--boolean_array=false",
		                      "--boolean_array=true"};

		r = cfgopt_args_parse(&cfg,
		                      sizeof(args) / sizeof(args[0]),
		                      args);

		assert(r.type == CFGOPT_OK);
		assert(cfg.cfg_boolean_array.len == 2);
		assert(!cfg.cfg_boolean_array.data[0]);
		assert(cfg.cfg_boolean_array.data[1]);
	}
	return 0;
}
