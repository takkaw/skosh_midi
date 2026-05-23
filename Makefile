CC=gcc
CFLAGS=-std=gnu11 -Wall -Wextra -Wpedantic -Werror -fanalyzer -Wconversion -Wsign-conversion
LIBS=-lasound

all:
	$(CC) $(CFLAGS) example.c $(LIBS)

format:
	clang-format -i *.c *.h

check:
	cppcheck --std=c11 --enable=all --suppress=missingIncludeSystem --suppress=unusedFunction skoshmidi.h

tidy:
	clang-tidy example.c -- -std=c11 -D_GNU_SOURCE -lasound
