CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O2 -g -pthread
TARGETS := atomic_counter cas_counter spinlock_cas compare3

.PHONY: all clean
all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)
