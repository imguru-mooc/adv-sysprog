CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O0 -g
TARGETS := hello hello_static auxv stack_layout ctor nostart

.PHONY: all clean
all: $(TARGETS)

hello_static: hello.c
	$(CC) $(CFLAGS) -static -o $@ $<

nostart: nostart.c
	$(CC) $(CFLAGS) -nostdlib -static -o $@ $<

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)
