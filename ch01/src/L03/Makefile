CC      := gcc
CFLAGS  := -Wall -Wextra -Wpedantic -O0 -g
TARGETS := sections sections_nopie sections_static big_bss big_data

.PHONY: all clean
all: $(TARGETS)

sections: sections.c
	$(CC) $(CFLAGS) -o $@ $<

# PIE 를 끈 실행 File (Type: EXEC, 고정 주소)
sections_nopie: sections.c
	$(CC) $(CFLAGS) -no-pie -fno-pie -o $@ $<

# Shared Library 없이 모든 코드를 포함한 실행 File
sections_static: sections.c
	$(CC) $(CFLAGS) -static -o $@ $<

big_bss: big_bss.c
	$(CC) $(CFLAGS) -o $@ $<

big_data: big_data.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGETS)
