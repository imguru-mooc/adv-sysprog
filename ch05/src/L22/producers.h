/* producers.h - 실습용 입력 두 개를 만든다
 *   fast: 0.2 초 뒤부터 0.5 초마다 4 번      slow: 1.2 초 뒤에 한 번
 * 각 Producer 는 다 보낸 뒤 Pipe 를 닫는다(EOF). return: 읽기용 fd 두 개
 */
#ifndef PRODUCERS_H
#define PRODUCERS_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static inline double now_s(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static inline int spawn_producer(const char *name, int first_ms, int period_ms, int count)
{
    char msg[64];
    int  pfd[2], i;

    if (pipe(pfd) < 0) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    if (fork() == 0) {
        close(pfd[0]);
        usleep((useconds_t)first_ms * 1000);
        for (i = 1; i <= count; i++) {
            int len = snprintf(msg, sizeof(msg), "%s#%d", name, i);

            if (write(pfd[1], msg, (size_t)len) < 0)
                _exit(EXIT_FAILURE);
            if (i < count)
                usleep((useconds_t)period_ms * 1000);
        }
        _exit(EXIT_SUCCESS);                /* 종료하면서 Pipe 의 쓰기 쪽이 닫힌다 → Reader 는 EOF */
    }
    close(pfd[1]);
    return pfd[0];
}

#endif
