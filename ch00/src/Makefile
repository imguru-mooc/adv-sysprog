CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O0 -g
# ltrace는 Lazy Binding(PLT)을 가로채므로 BIND_NOW가 기본인 최신 Ubuntu에서는 -z lazy가 필요하다 (L04, L32에서 설명)
LDFLAGS := -Wl,-z,lazy

TARGET  := hello_trace

.PHONY: all clean check
all: $(TARGET)

$(TARGET): hello_trace.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

check: $(TARGET)
	@echo "--- strace"
	strace -e trace=write ./$(TARGET)
	@echo "--- perf stat"
	perf stat -e page-faults,context-switches ./$(TARGET)

clean:
	rm -f $(TARGET)
