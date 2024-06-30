main: main.c cfgopt_gen.h
	cc -Wall -Wextra -Werror -g -o main main.c

cfgopt_gen.h: templates/c.txt src/main.rs cfgopt.toml
	cargo r
