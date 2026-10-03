CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O0 -g
TARGETS := libgreet.so main main_rpath main_lazy main_now libhook.so dice plugin_main

.PHONY: all clean
all: $(TARGETS)

# Shared Library: 위치 독립 코드(-fPIC) + -shared
libgreet.so: greet.c greet.h
	$(CC) $(CFLAGS) -fPIC -shared -o $@ greet.c

libhook.so: hook.c
	$(CC) $(CFLAGS) -fPIC -shared -o $@ hook.c -ldl

# 1) 기본: 실행 시 libgreet.so 위치를 모른다 (LD_LIBRARY_PATH 필요)
main: main.c libgreet.so
	$(CC) $(CFLAGS) -o $@ main.c -L. -lgreet

# 2) RUNPATH 기록: 실행 File 과 같은 Directory 에서 찾는다
main_rpath: main.c libgreet.so
	$(CC) $(CFLAGS) -o $@ main.c -L. -lgreet -Wl,-rpath,'$$ORIGIN'

# 3) Lazy Binding 관찰용: 고정 주소(-no-pie) + 첫 호출 때 주소 결정(-z lazy)
main_lazy: main.c libgreet.so
	$(CC) $(CFLAGS) -no-pie -fno-pie -o $@ main.c -L. -lgreet -Wl,-rpath,'$$ORIGIN' -Wl,-z,lazy -Wl,-z,norelro

# 4) 비교용: 시작할 때 모든 주소 결정(-z now). 최신 Ubuntu 의 기본 동작
main_now: main.c libgreet.so
	$(CC) $(CFLAGS) -no-pie -fno-pie -o $@ main.c -L. -lgreet -Wl,-rpath,'$$ORIGIN' -Wl,-z,now

dice: dice.c
	$(CC) $(CFLAGS) -o $@ dice.c

plugin_main: plugin_main.c libgreet.so
	$(CC) $(CFLAGS) -o $@ plugin_main.c -ldl

clean:
	rm -f $(TARGETS)
