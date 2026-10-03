/* eventfd_worker.c - Worker Thread 가 "일이 끝났다" 를 eventfd 로 알리고, main 은 epoll 로 기다린다 */
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>

static int efd;

static void *worker(void *arg)
{
    uint64_t one = 1;
    int      i;

    (void)arg;
    for (i = 1; i <= 5; i++) {
        usleep(100 * 1000);                             /* 0.1 초짜리 작업 */
        if (write(efd, &one, sizeof(one)) != sizeof(one))   /* Counter += 1. 반드시 8 Byte */
            perror("write(eventfd)");
    }
    return NULL;
}

int main(void)
{
    struct epoll_event ev, out;
    pthread_t          tid;
    uint64_t           count, total = 0;
    int                epfd;

    efd  = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);      /* Kernel 안의 64bit Counter 하나 */
    epfd = epoll_create1(EPOLL_CLOEXEC);
    if (efd < 0 || epfd < 0)
        return EXIT_FAILURE;
    ev.events  = EPOLLIN;
    ev.data.fd = efd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, efd, &ev);

    pthread_create(&tid, NULL, worker, NULL);

    while (total < 5) {
        if (epoll_wait(epfd, &out, 1, -1) < 1)
            continue;
        usleep(250 * 1000);                             /* main 이 다른 일로 바쁜 상황을 흉내 */
        if (read(efd, &count, sizeof(count)) != sizeof(count))  /* Counter 를 읽고 0 으로 Reset */
            continue;
        total += count;
        printf("eventfd 에서 %llu 을(를) 읽음 → 완료된 작업 누계 %llu\n",
               (unsigned long long)count, (unsigned long long)total);
    }
    pthread_join(tid, NULL);
    return EXIT_SUCCESS;
}
