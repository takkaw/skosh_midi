CC=gcc
CFLAGS=-std=gnu11 -Wall -Wextra -Wpedantic -Werror -fanalyzer -Wconversion -Wsign-conversion
LIBS=-lasound

all:
	$(CC) $(CFLAGS) example.c $(LIBS)

check:
	cppcheck --std=c11 --enable=all --suppress=missingIncludeSystem --suppress=unusedFunction skoshmidi.h

format:
	clang-format -i *.c *.h
