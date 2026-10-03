CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O2 -g -fno-omit-frame-pointer -pthread
LDLIBS  := -lcap -lseccomp
TARGETS := monitor logdump load

.PHONY: all clean
all: $(TARGETS)

monitor: monitor.c procstat.h mlog.h harden.h
	$(CC) $(CFLAGS) -o $@ monitor.c $(LDLIBS)

logdump: logdump.c mlog.h
	$(CC) $(CFLAGS) -o $@ logdump.c

load: load.c
	$(CC) $(CFLAGS) -o $@ load.c

clean:
	rm -f $(TARGETS) monitor.log.bin perf.data perf.data.old
