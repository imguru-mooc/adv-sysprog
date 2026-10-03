CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O2 -g
TARGETS := same_api fs_type inode_life

.PHONY: all clean
all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS) life.txt life2.txt life3.txt
