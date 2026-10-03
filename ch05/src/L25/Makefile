CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O2 -g -pthread
TARGETS := eventfd_worker timerfd_demo signalfd_demo inotify_demo event_loop

.PHONY: all clean
all: $(TARGETS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)
	rm -rf watched
