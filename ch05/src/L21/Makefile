CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O2 -g 
TARGETS := block_read nonblock_read pipe_full two_pipes

.PHONY: all clean
all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)
