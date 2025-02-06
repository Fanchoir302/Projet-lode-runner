CFLAGS=-Wall -pedantic -std=c99 -fsanitize=address,undefined

build: lode_runner.c player.c
	gcc -Wall -Wextra -fsanitize=address,undefined -g -o lode_runner lode_runner.o int_list.c player.c -lm

