CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O2 -g -pthread
TARGETS := false_sharing lock_scale lockfree_stack

.PHONY: all clean
all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)
