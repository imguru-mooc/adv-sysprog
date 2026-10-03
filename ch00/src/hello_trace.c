/*
 * hello_trace.c - 관찰 도구 동작 확인용 프로그램
 *
 * 하는 일은 단순하다. 중요한 것은 이 프로그램을
 * strace / ltrace / perf / bpftrace 로 "관찰"해 보는 것이다.
 */
#define _GNU_SOURCE
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PAGES 256

int main(void)
{
    const char msg[] = "hello, kernel\n";
    long page = sysconf(_SC_PAGESIZE);
    char *buf;
    ssize_t n;

    if (page < 0) {
        perror("sysconf");
        return EXIT_FAILURE;
    }

    /* 1. System Call 한 번: strace에서 write(1, ...) 로 보인다 */
    n = write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    if (n < 0) {
        perror("write");
        return EXIT_FAILURE;
    }

    /* 2. Library 함수: ltrace에서 malloc, memset 으로 보인다 */
    buf = malloc((size_t)page * PAGES);
    if (buf == NULL) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    /* 3. Page를 처음 건드리는 순간 Page Fault: perf stat의 page-faults 로 보인다 */
    memset(buf, 1, (size_t)page * PAGES);

    printf("pid=%d page=%ld touched=%d pages\n", (int)getpid(), page, PAGES);

    free(buf);
    return EXIT_SUCCESS;
}
