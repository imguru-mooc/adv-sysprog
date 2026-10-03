CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O2 -g
TARGETS := strict rawbpf sandbox

.PHONY: all clean
all: $(TARGETS)

strict rawbpf: %: %.c
	$(CC) $(CFLAGS) -o $@ $<

sandbox: sandbox.c
	$(CC) $(CFLAGS) -o $@ $< -lseccomp

clean:
	rm -f $(TARGETS) sandbox.out
