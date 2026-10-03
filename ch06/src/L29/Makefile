CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O2 -g
LDLIBS  := -luring
TARGETS := uring_cat batch_compare uring_cp

.PHONY: all clean
all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

clean:
	rm -f $(TARGETS) *.dat
