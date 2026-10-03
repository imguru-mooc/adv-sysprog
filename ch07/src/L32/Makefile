CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O0 -g -pthread
TARGETS := chatty crash deadlock

.PHONY: all clean
all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS) core core.* chatty.out *.txt
