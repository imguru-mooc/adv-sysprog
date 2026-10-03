CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O0 -g -pthread
TARGETS := race_counter race_mutex check_then_act

.PHONY: all clean tsan
all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

# ThreadSanitizer 로 Data Race 를 검출하는 Build
tsan: race_counter.c
	$(CC) -Wall -Wextra -O1 -g -pthread -fsanitize=thread -o race_counter_tsan $<

clean:
	rm -f $(TARGETS) race_counter_tsan race_counter_O2
