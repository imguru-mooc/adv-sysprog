CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O2 -g -fno-omit-frame-pointer -fno-ipa-cp -fno-optimize-sibling-calls
TARGETS := chatty hot

.PHONY: all clean
all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $< -lm

clean:
	rm -f $(TARGETS) chatty.out
