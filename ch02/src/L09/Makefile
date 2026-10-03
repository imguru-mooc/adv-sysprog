CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O1 -g
TARGETS := fault_count zero_page major_fault cow_fault segv_info

.PHONY: all clean
all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS) big.dat
