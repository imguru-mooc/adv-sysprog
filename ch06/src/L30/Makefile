CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O2 -g
LDLIBS  := -luring
TARGETS := uring_eventfd uring_sqpoll uring_fixed

.PHONY: all clean
all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

clean:
	rm -f $(TARGETS) *.dat
