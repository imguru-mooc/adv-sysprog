CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O2 -g
TARGETS := epoll_demo scale_compare epoll_inspect

.PHONY: all clean
all: $(TARGETS)

%: %.c producers.h
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)
