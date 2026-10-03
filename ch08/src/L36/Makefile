CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O2 -g
LDLIBS  := -lcap
TARGETS := capshow capdemo dropcaps

.PHONY: all clean
all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

clean:
	rm -f $(TARGETS)
