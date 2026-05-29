CC=gcc
CFLAGS=-std=gnu11 -Wall -Wextra -Wpedantic -Werror -fanalyzer -Wconversion -Wsign-conversion
LIBS=-lasound

all:
	$(CC) $(CFLAGS) -o example example.c $(LIBS)

format:
	clang-format -i *.c *.h

check:
	cppcheck \
	--std=c11 --enable=all \
	--suppress=missingIncludeSystem --suppress=unusedFunction \
	skosh_midi.h example.c

tidy:
	clang-tidy \
	-checks=-clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling \
 	skosh_midi.h example.c \
 	-- -std=c11 -D_GNU_SOURCE -lasound
