CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O1 -g
TARGETS := va_split pagemap vmpte cow_pfn

.PHONY: all clean
all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS) *.dat
