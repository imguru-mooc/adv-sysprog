/* vdso_time.c - 같은 clock_gettime 을 vDSO 경로와 실제 System Call 경로로 호출해 비교한다 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#define LOOPS 1000000L

static double now_sec(void)
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

int main(void)
{
    struct timespec ts;
    double t0, t1, t2;
    long i;

    t0 = now_sec();
    for (i = 0; i < LOOPS; i++)
        clock_gettime(CLOCK_MONOTONIC, &ts);                 /* glibc -> vDSO */
    t1 = now_sec();
    for (i = 0; i < LOOPS; i++)
        syscall(SYS_clock_gettime, CLOCK_MONOTONIC, &ts);    /* 실제 Kernel 진입 */
    t2 = now_sec();

    printf("vDSO    : %8.1f ns/call\n", (t1 - t0) * 1e9 / (double)LOOPS);
    printf("syscall : %8.1f ns/call\n", (t2 - t1) * 1e9 / (double)LOOPS);
    printf("ratio   : %8.1f x\n", (t2 - t1) / (t1 - t0));
    return 0;
}
