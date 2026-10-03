/* scale_compare.c - fd N 개 중 1 개만 활동할 때 poll 과 epoll 의 호출당 비용을 비교한다
 * 사용법: ./scale_compare [N ...]      기본: 10 100 1000 10000
 */
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>

#define ROUNDS 2000

static double now_us(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e6 + (double)ts.tv_nsec / 1e3;
}

static void run(int n)
{
    struct pollfd     *pfds = calloc((size_t)n, sizeof(*pfds));
    struct epoll_event ev, out[16];
    uint64_t           one = 1, val;
    double             t0, t1, t2;
    int                epfd = epoll_create1(0), i, r, k;

    if (pfds == NULL || epfd < 0)
        exit(EXIT_FAILURE);
    for (i = 0; i < n; i++) {
        pfds[i].fd = eventfd(0, EFD_NONBLOCK);
        pfds[i].events = POLLIN;
        if (pfds[i].fd < 0) {
            perror("eventfd (ulimit -n 을 확인하세요)");
            exit(EXIT_FAILURE);
        }
        ev.events = EPOLLIN;
        ev.data.fd = pfds[i].fd;
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, pfds[i].fd, &ev) < 0)    /* 등록은 측정 밖에서 한 번 */
            exit(EXIT_FAILURE);
    }

    t0 = now_us();
    for (r = 0; r < ROUNDS; r++) {
        if (write(pfds[n - 1].fd, &one, sizeof(one)) < 0)
            exit(EXIT_FAILURE);
        if (poll(pfds, (nfds_t)n, -1) != 1)
            exit(EXIT_FAILURE);
        for (i = 0; i < n; i++)
            if ((pfds[i].revents & POLLIN) && read(pfds[i].fd, &val, sizeof(val)) < 0)
                exit(EXIT_FAILURE);
    }
    t1 = now_us();
    for (r = 0; r < ROUNDS; r++) {
        if (write(pfds[n - 1].fd, &one, sizeof(one)) < 0)
            exit(EXIT_FAILURE);
        k = epoll_wait(epfd, out, 16, -1);
        for (i = 0; i < k; i++)
            if (read(out[i].data.fd, &val, sizeof(val)) < 0)
                exit(EXIT_FAILURE);
    }
    t2 = now_us();

    printf("%8d %14.1f %14.1f %9.0fx\n", n, (t1 - t0) / ROUNDS, (t2 - t1) / ROUNDS, (t1 - t0) / (t2 - t1));
    for (i = 0; i < n; i++)
        close(pfds[i].fd);
    close(epfd);
    free(pfds);
}

int main(int argc, char *argv[])
{
    struct rlimit rl;
    int           defaults[] = { 10, 100, 1000, 10000 }, i;

    if (getrlimit(RLIMIT_NOFILE, &rl) == 0) {
        rl.rlim_cur = rl.rlim_max;
        setrlimit(RLIMIT_NOFILE, &rl);
    }
    printf("%8s %14s %14s %10s\n", "N", "poll (us)", "epoll (us)", "poll/epoll");
    if (argc > 1)
        for (i = 1; i < argc; i++)
            run(atoi(argv[i]));
    else
        for (i = 0; i < 4; i++)
            run(defaults[i]);
    return EXIT_SUCCESS;
}
