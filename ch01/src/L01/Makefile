CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O0 -g
LDFLAGS := -Wl,-z,lazy
TARGETS := hello1_printf hello2_write hello3_syscall libonly priv errno_demo

.PHONY: all clean
all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(TARGETS)
