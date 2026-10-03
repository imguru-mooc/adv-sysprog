/* burn.c - 지정한 시간(벽시계) 동안 쉬지 않고 CPU 를 쓰려고 시도하고, 실제로 받은 CPU 시간을 보고한다
 * 사용법: ./burn [초=5]
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static double clk(clockid_t id)
{
    struct timespec ts;

    clock_gettime(id, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

int main(int argc, char *argv[])
{
    double            secs = argc > 1 ? atof(argv[1]) : 5.0;
    double            w0 = clk(CLOCK_MONOTONIC), c0 = clk(CLOCK_PROCESS_CPUTIME_ID), next = 1.0, w, c;
    volatile unsigned long x = 0;

    while ((w = clk(CLOCK_MONOTONIC) - w0) < secs) {
        x++;
        if (w >= next) {
            c = clk(CLOCK_PROCESS_CPUTIME_ID) - c0;
            printf("[%2.0f s] 받은 CPU 시간 %.2f s  → %5.1f %%\n", next, c, 100.0 * c / w);
            fflush(stdout);
            next += 1.0;
        }
    }
    c = clk(CLOCK_PROCESS_CPUTIME_ID) - c0;
    printf("합계: 벽시계 %.2f s 동안 CPU %.2f s (%.1f %%)\n", w, c, 100.0 * c / w);
    return EXIT_SUCCESS;
}
