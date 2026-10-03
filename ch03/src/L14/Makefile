CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O2 -g
TARGETS := io_compare direct_align rand_compare

.PHONY: all clean
all: $(TARGETS)

%: %.c cachelib.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS) *.dat
