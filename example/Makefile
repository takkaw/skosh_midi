CC=gcc
CFLAGS=-std=gnu11 -Wall -Wextra -Wpedantic -Werror -Wconversion -Wsign-conversion -I..
TARGET=skosh_midi_example

UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
	CFLAGS += -fanalyzer
	LDFLAGS += -lasound
endif
ifeq ($(UNAME), Darwin)
	LDFLAGS += -framework CoreFoundation -framework CoreMIDI
endif
ifneq ($(filter MINGW% MSYS% CYGWIN%,$(UNAME)),)
	LDFLAGS += -lwinmm
endif

all:
	$(CC) $(CFLAGS) -o $(TARGET) example_main.c check_include.c $(LDFLAGS)

clean:
	rm -f $(TARGET)

format:
	clang-format -i *.c ../*.h

check:
	cppcheck \
	--std=c11 --enable=all \
	--suppress=missingIncludeSystem --suppress=unusedFunction \
	example_main.c check_include.c -I..

tidy:
	clang-tidy \
	-checks='clang-analyzer-*,bugprone-*,performance-*,cert-*,misc--*,\
	-clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling,\
	-bugprone-easily-swappable-parameters' \
	example_main.c check_include.c \
	-- -std=c11 -D_GNU_SOURCE -I..
