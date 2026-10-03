CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O2 -g
TARGETS := select_demo poll_demo select_limit scale_poll

.PHONY: all clean
all: $(TARGETS)

%: %.c producers.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)
