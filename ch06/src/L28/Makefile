CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O2 -g
LDLIBS  := -luring
TARGETS := ring_inspect out_of_order ring_limits

.PHONY: all clean
all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

clean:
	rm -f $(TARGETS)
