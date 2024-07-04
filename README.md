# cfgopt - Multi-language command line parser generator based on config file

## What is `cfgopt`

`cfgopt` read a config file, which defined interface of command line argument
for you application, and generate code for different programming languages to
parse that command line argument.

## How to use now

`cfgopt` is at a early stage. There is only a C generator.

	cargo r -- -lc lib/c/cfgopt_gen.h

This read "./cfgopt.toml" and generate "lib/c/cfgopt_gen.h". Include this file
to your application to parse command line arguments. lib/c/cfgopt_test.c is a
example to use that parser.
