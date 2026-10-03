CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O2 -g -pthread
TARGETS := stop_flag stop_flag_atomic store_buffer store_buffer_fixed

.PHONY: all clean
all: $(TARGETS)

stop_flag_atomic: stop_flag.c
	$(CC) $(CFLAGS) -DUSE_ATOMIC -o $@ $<

store_buffer_fixed: store_buffer.c
	$(CC) $(CFLAGS) -DUSE_ATOMIC -o $@ $<

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)
