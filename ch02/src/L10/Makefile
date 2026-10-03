CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O1 -g
TARGETS := mmap_cat mmap_shared mmap_private anon_shared sigbus

.PHONY: all clean
all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS) note.txt bus.dat
