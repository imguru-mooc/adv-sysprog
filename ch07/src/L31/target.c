/* target.c - 관찰 대상 프로그램. 일부러 여러 가지 일을 한다
 *   Thread "burner" : 1 초 중 0.3 초 동안 CPU 를 쓴다
 *   Thread "logger" : 0.1 초마다 target.log 에 한 줄 쓴다
 *   main            : 2 초마다 1MB 를 malloc 하고 채운다 (최대 64MB), 나머지는 sleep
 */
#define _GNU_SOURCE
#include <fcntl.h>
#include <pthread.h>
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

static void *burner(void *arg)
{
    volatile unsigned long x = 0;

    (void)arg;
    pthread_setname_np(pthread_self(), "burner");       /* /proc/PID/task/TID/comm 에 나타난다 */
    for (;;) {
        double t0 = now();

        while (now() - t0 < 0.3)
            x++;
        usleep(700 * 1000);
    }
    return NULL;
}

static void *logger(void *arg)
{
    char line[64];
    int  n = 0, fd = open("target.log", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    (void)arg;
    pthread_setname_np(pthread_self(), "logger");
    for (;;) {
        int len = snprintf(line, sizeof(line), "line %d\n", ++n);

        if (fd < 0 || write(fd, line, (size_t)len) < 0)
            break;
        usleep(100 * 1000);
    }
    return NULL;
}

int main(void)
{
    pthread_t t1, t2;
    int       i;

    printf("target pid = %d\n", (int)getpid());
    fflush(stdout);
    pthread_create(&t1, NULL, burner, NULL);
    pthread_create(&t2, NULL, logger, NULL);
    for (i = 0; ; i++) {
        if (i < 64) {
            char *p = malloc(1 << 20);

            if (p != NULL)
                memset(p, 1, 1 << 20);                  /* 실제로 써야 RSS 가 늘어난다 (L09) */
        }
        sleep(2);
    }
    return EXIT_SUCCESS;
}
