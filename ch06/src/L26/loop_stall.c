/* loop_stall.c - Event Loop 안에서 File 을 읽으면 Loop 전체가 멈춘다
 *   10ms 주기 timerfd 로 "심장 박동" 을 찍는다. 3 번째 Tick 에서 Cache 를 비운 256MB File 을 read 한다.
 *   read 가 Disk 를 기다리는 동안 놓친 Tick 수가 timerfd 의 만료 횟수로 드러난다.
 */
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>

#define NAME "big.dat"
#define MB   256

static char buf[1 << 20];

static double now_ms(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e3 + (double)ts.tv_nsec / 1e6;
}

static void prepare(void)
{
    int i, fd;

    if (access(NAME, F_OK) != 0) {
        printf("%s (%dMB) 생성 중...\n", NAME, MB);
        fd = open(NAME, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        memset(buf, 'x', sizeof(buf));
        for (i = 0; i < MB; i++)
            if (write(fd, buf, sizeof(buf)) < 0)
                exit(EXIT_FAILURE);
        fsync(fd);
        close(fd);
    }
    fd = open(NAME, O_RDONLY);
    fsync(fd);
    posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);       /* 이 File 의 Page Cache 를 비운다 (L13) */
    close(fd);
}

int main(void)
{
    struct itimerspec  its = { { 0, 10 * 1000 * 1000 }, { 0, 10 * 1000 * 1000 } };     /* 10ms */
    struct epoll_event ev = { .events = EPOLLIN }, out;
    uint64_t           exp;
    double             t0, t1;
    int                tfd, epfd, tick, fd;
    ssize_t            n;

    prepare();
    tfd  = timerfd_create(CLOCK_MONOTONIC, 0);
    epfd = epoll_create1(0);
    timerfd_settime(tfd, 0, &its, NULL);
    epoll_ctl(epfd, EPOLL_CTL_ADD, tfd, &ev);
    t0 = now_ms();

    for (tick = 1; tick <= 6; tick++) {
        epoll_wait(epfd, &out, 1, -1);
        if (read(tfd, &exp, sizeof(exp)) != sizeof(exp))
            continue;
        printf("[%7.1f ms] tick %d  (만료 횟수 %llu)%s\n", now_ms() - t0, tick, (unsigned long long)exp,
               exp > 1 ? "   ← 그동안 Loop 가 멈춰 있었다" : "");
        if (tick == 3) {                                /* Handler 안에서 File 을 읽는다 */
            t1 = now_ms();
            fd = open(NAME, O_RDONLY | O_NONBLOCK);     /* O_NONBLOCK 은 아무 효과가 없다 */
            while ((n = read(fd, buf, sizeof(buf))) > 0)
                ;
            close(fd);
            printf("             Handler: %dMB read 에 %.1f ms\n", MB, now_ms() - t1);
        }
    }
    return EXIT_SUCCESS;
}
