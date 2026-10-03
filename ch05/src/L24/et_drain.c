/* et_drain.c - Edge Trigger 의 올바른 사용법: Non-blocking fd + EAGAIN 이 나올 때까지 읽는다 */
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <unistd.h>

int main(void)
{
    struct epoll_event ev, out;
    char               buf[4];
    int                pfd[2], epfd, round, n, reads;
    ssize_t            got;

    if (pipe2(pfd, O_NONBLOCK) < 0 || (epfd = epoll_create1(0)) < 0)    /* ET 는 반드시 O_NONBLOCK 과 함께 */
        return EXIT_FAILURE;
    ev.events  = EPOLLIN | EPOLLET;
    ev.data.fd = pfd[0];
    epoll_ctl(epfd, EPOLL_CTL_ADD, pfd[0], &ev);

    if (write(pfd[1], "HELLO LINUX", 11) != 11)
        return EXIT_FAILURE;

    for (round = 1; round <= 2; round++) {
        n = epoll_wait(epfd, &out, 1, 1000);
        if (n == 0) {
            printf("epoll_wait #%d: Timeout — 남은 Data 가 없으므로 정상\n", round);
            continue;
        }
        printf("epoll_wait #%d: EPOLLIN\n", round);
        reads = 0;
        for (;;) {                                      /* 비울 때까지 읽는다 */
            got = read(pfd[0], buf, 3);
            reads++;
            if (got > 0) {
                buf[got] = '\0';
                printf("   read #%d: \"%s\"\n", reads, buf);
            } else if (got < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                printf("   read #%d: EAGAIN → 다 읽었다. epoll_wait 로 돌아간다\n", reads);
                break;
            } else {
                break;                                  /* 0: EOF, 그 외: 오류 */
            }
        }
    }
    return EXIT_SUCCESS;
}
