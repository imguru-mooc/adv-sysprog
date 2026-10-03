CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O2 -g
TARGETS := bufsize fd_share short_io

.PHONY: all clean
all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS) *.dat
