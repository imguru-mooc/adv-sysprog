CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O1 -g
TARGETS := addr_map heap_grow stack_grow vsz_rss

.PHONY: all clean
all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS) *.dat
