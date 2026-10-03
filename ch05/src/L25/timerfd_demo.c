/* timerfd_demo.c - Timer 가 fd 가 된다: 만료되면 읽을 수 있게 되고, epoll 로 기다릴 수 있다 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

static double now(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

int main(void)
{
    struct itimerspec  its = { { 0, 200 * 1000 * 1000 }, { 0, 500 * 1000 * 1000 } };   /* 0.5 초 뒤 시작, 0.2 초 주기 */
    struct epoll_event ev, out;
    uint64_t           expirations;
    double             t0 = now();
    int                tfd, epfd, i;

    tfd  = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    epfd = epoll_create1(EPOLL_CLOEXEC);
    if (tfd < 0 || epfd < 0 || timerfd_settime(tfd, 0, &its, NULL) < 0) {
        perror("timerfd");
        return EXIT_FAILURE;
    }
    ev.events  = EPOLLIN;
    ev.data.fd = tfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, tfd, &ev);

    for (i = 1; i <= 5; i++) {
        epoll_wait(epfd, &out, 1, -1);
        if (i == 3)
            usleep(700 * 1000);                         /* 한 번 늦게 읽어 본다 */
        if (read(tfd, &expirations, sizeof(expirations)) != sizeof(expirations))
            continue;
        printf("[%4.2fs] Timer Event, 그동안의 만료 횟수 = %llu\n", now() - t0, (unsigned long long)expirations);
    }
    return EXIT_SUCCESS;
}
