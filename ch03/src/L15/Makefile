CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O2 -g
TARGETS := dirty_watch sync_cost safe_save

.PHONY: all clean
all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS) *.dat config.txt config.txt.tmp
