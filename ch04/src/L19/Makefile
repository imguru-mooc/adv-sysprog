CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O2 -g -pthread
TARGETS := mutex_nocontend mutex_contend futex_basic my_mutex

.PHONY: all clean
all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)
