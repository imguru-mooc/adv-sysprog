/* load.c - monitor 로 관찰할 부하 생성기: 1 초 중 지정한 비율만큼 CPU 를 쓰고, 5 초마다 8MB 씩 Memory 를 늘린다
 * 사용법: ./load [CPU%=40]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static double now(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

int main(int argc, char *argv[])
{
    double            duty = (argc > 1 ? atof(argv[1]) : 40.0) / 100.0;
    volatile unsigned long x = 0;
    int               sec;

    printf("load pid = %d (CPU %.0f%%)\n", (int)getpid(), duty * 100);
    fflush(stdout);
    for (sec = 1; ; sec++) {
        double t0 = now();

        while (now() - t0 < duty)
            x++;
        usleep((useconds_t)((1.0 - duty) * 1e6));
        if (sec % 5 == 0 && sec <= 60) {
            char *p = malloc(8u << 20);

            if (p != NULL)
                memset(p, 1, 8u << 20);
        }
    }
    return EXIT_SUCCESS;
}
