/* scale_poll.c - 감시하는 fd 가 N 개이고 그중 1 개만 활동할 때, poll() 한 번의 비용
 * 사용법: ./scale_poll [N ...]        기본: 10 100 1000 10000
 */
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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
    struct pollfd *pfds = calloc((size_t)n, sizeof(*pfds));
    uint64_t       one = 1, val;
    double         t0, t1;
    int            i, r;

    if (pfds == NULL)
        exit(EXIT_FAILURE);
    for (i = 0; i < n; i++) {
        pfds[i].fd = eventfd(0, EFD_NONBLOCK);      /* 가벼운 fd 를 N 개 만든다 */
        pfds[i].events = POLLIN;
        if (pfds[i].fd < 0) {
            perror("eventfd (ulimit -n 을 확인하세요)");
            exit(EXIT_FAILURE);
        }
    }
    t0 = now_us();
    for (r = 0; r < ROUNDS; r++) {
        if (write(pfds[n - 1].fd, &one, sizeof(one)) < 0)       /* 마지막 fd 하나만 활동 */
            exit(EXIT_FAILURE);
        if (poll(pfds, (nfds_t)n, -1) != 1)
            exit(EXIT_FAILURE);
        for (i = 0; i < n; i++)                                 /* 누구인지 찾으려면 전부 본다 */
            if (pfds[i].revents & POLLIN)
                if (read(pfds[i].fd, &val, sizeof(val)) < 0)
                    exit(EXIT_FAILURE);
    }
    t1 = now_us();
    printf("N = %6d   poll 1 회 (Event 1 개 처리 포함) 평균 %9.1f us\n", n, (t1 - t0) / ROUNDS);
    for (i = 0; i < n; i++)
        close(pfds[i].fd);
    free(pfds);
}

int main(int argc, char *argv[])
{
    struct rlimit rl;
    int           defaults[] = { 10, 100, 1000, 10000 }, i;

    if (getrlimit(RLIMIT_NOFILE, &rl) == 0) {       /* Soft Limit 을 Hard Limit 까지 올린다 */
        rl.rlim_cur = rl.rlim_max;
        setrlimit(RLIMIT_NOFILE, &rl);
    }
    if (argc > 1)
        for (i = 1; i < argc; i++)
            run(atoi(argv[i]));
    else
        for (i = 0; i < 4; i++)
            run(defaults[i]);
    return EXIT_SUCCESS;
}
