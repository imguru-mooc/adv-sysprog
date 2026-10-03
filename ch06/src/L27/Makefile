CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O2 -g
LDLIBS  := -luring
TARGETS := uring_hello uring_raw uring_probe

.PHONY: all clean
all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

clean:
	rm -f $(TARGETS)
