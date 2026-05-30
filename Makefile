CC=gcc
CFLAGS=-std=gnu11 -Wall -Wextra -Wpedantic -Werror -fanalyzer -Wconversion -Wsign-conversion
LIBS=-lasound
TARGET=example

all:
	$(CC) $(CFLAGS) -o $(TARGET) example.c $(LIBS)

clean:
	rm -f $(TARGET)

format:
	clang-format -i *.c *.h

check:
	cppcheck \
	--std=c11 --enable=all \
	--suppress=missingIncludeSystem --suppress=unusedFunction \
	skosh_midi.h example.c

tidy:
	clang-tidy \
	-checks='clang-analyzer-*,bugprone-*,performance-*,cert-*,misc--*,-clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling' \
	skosh_midi.h example.c \
 	-- -std=c11 -D_GNU_SOURCE -lasound
