CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O1 -g
TARGETS := raw_syscall vdso_time pause_write

.PHONY: all clean
all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)
