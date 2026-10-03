/* event_loop.c - 하나의 epoll Loop 에서 Timer, Signal, Worker Thread, File 변화를 모두 처리한다
 * Final Project(Linux Event & Process Monitor) 의 뼈대
 *   1 초마다 Timer Tick / Worker 가 0.7 초마다 작업 완료 통지 / watched/ 의 File 변화 / Ctrl+C 로 Graceful Shutdown
 */
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/inotify.h>
#include <sys/signalfd.h>
#include <sys/stat.h>
#include <sys/timerfd.h>
#include <unistd.h>

enum { SRC_TIMER, SRC_SIGNAL, SRC_WORKER, SRC_INOTIFY };        /* epoll_event.data 에 담아 둘 출처 표시 */

static int         efd;
static atomic_bool stop_worker;

static void *worker(void *arg)
{
    uint64_t one = 1;

    (void)arg;
    while (!atomic_load(&stop_worker)) {
        usleep(700 * 1000);
        if (write(efd, &one, sizeof(one)) < 0)
            break;
    }
    return NULL;
}

static void add(int epfd, int fd, uint32_t src)
{
    struct epoll_event ev;

    ev.events   = EPOLLIN;
    ev.data.u32 = src;                                          /* fd 대신 "무엇인지" 를 저장 */
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) < 0) {
        perror("epoll_ctl");
        exit(EXIT_FAILURE);
    }
}

int main(void)
{
    struct itimerspec       its = { { 1, 0 }, { 1, 0 } };
    struct signalfd_siginfo si;
    struct epoll_event      events[8];
    char                    ibuf[4096];
    pthread_t               tid;
    sigset_t                mask;
    uint64_t                v, ticks = 0, jobs = 0;
    int                     epfd, tfd, sfd, ifd, running = 1, i, n;

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);                    /* Thread 생성 전에 Block: 모든 Thread 가 상속 */

    epfd = epoll_create1(EPOLL_CLOEXEC);
    tfd  = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    sfd  = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
    efd  = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    ifd  = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    mkdir("watched", 0755);
    if (epfd < 0 || tfd < 0 || sfd < 0 || efd < 0 || ifd < 0 ||
        timerfd_settime(tfd, 0, &its, NULL) < 0 ||
        inotify_add_watch(ifd, "watched", IN_CREATE | IN_DELETE | IN_CLOSE_WRITE) < 0) {
        perror("setup");
        return EXIT_FAILURE;
    }
    add(epfd, tfd, SRC_TIMER);
    add(epfd, sfd, SRC_SIGNAL);
    add(epfd, efd, SRC_WORKER);
    add(epfd, ifd, SRC_INOTIFY);
    pthread_create(&tid, NULL, worker, NULL);
    printf("pid %d: Event Loop 시작. 다른 Terminal 에서 touch watched/x, 종료는 Ctrl+C\n", (int)getpid());

    while (running) {
        n = epoll_wait(epfd, events, 8, -1);                    /* 프로그램 전체에서 기다리는 곳은 여기 하나 */
        for (i = 0; i < n; i++) {
            switch (events[i].data.u32) {
            case SRC_TIMER:
                if (read(tfd, &v, sizeof(v)) == sizeof(v)) {
                    ticks += v;
                    printf("[timer ] tick %llu  (완료된 작업 %llu)\n", (unsigned long long)ticks, (unsigned long long)jobs);
                }
                break;
            case SRC_WORKER:
                if (read(efd, &v, sizeof(v)) == sizeof(v)) {
                    jobs += v;
                    printf("[worker] 작업 %llu 개 완료 통지\n", (unsigned long long)v);
                }
                break;
            case SRC_INOTIFY:
                while (read(ifd, ibuf, sizeof(ibuf)) > 0)
                    printf("[file  ] watched/ 에 변화가 있습니다\n");
                break;
            case SRC_SIGNAL:
                while (read(sfd, &si, sizeof(si)) == (ssize_t)sizeof(si)) {
                    printf("[signal] %u 수신 → Graceful Shutdown\n", si.ssi_signo);
                    running = 0;
                }
                break;
            }
        }
    }
    atomic_store(&stop_worker, 1);                              /* 정리: Worker 종료 대기, fd 닫기 */
    pthread_join(tid, NULL);
    close(tfd); close(sfd); close(efd); close(ifd); close(epfd);
    printf("정상 종료: tick %llu, 작업 %llu\n", (unsigned long long)ticks, (unsigned long long)jobs);
    return EXIT_SUCCESS;
}
