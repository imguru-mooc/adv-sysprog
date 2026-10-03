/* lt_et.c - Pipe 에 "HELLO LINUX"(11 Byte) 가 들어 있다. Event 한 번에 3 Byte 씩만 읽는다
 * 사용법: ./lt_et lt      Level Trigger (기본 동작)
 *         ./lt_et et      Edge Trigger  (EPOLLET)
 */
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    struct epoll_event ev, out;
    char               buf[4];
    int                et = (argc > 1 && strcmp(argv[1], "et") == 0);
    int                pfd[2], epfd, round, n, pending;
    ssize_t            got;

    if (pipe2(pfd, O_NONBLOCK) < 0 || (epfd = epoll_create1(0)) < 0)
        return EXIT_FAILURE;
    ev.events  = EPOLLIN | (et ? EPOLLET : 0);
    ev.data.fd = pfd[0];
    epoll_ctl(epfd, EPOLL_CTL_ADD, pfd[0], &ev);

    if (write(pfd[1], "HELLO LINUX", 11) != 11)
        return EXIT_FAILURE;
    printf("Mode: %s,  Pipe 에 11 Byte 를 썼습니다\n", et ? "Edge Trigger (EPOLLET)" : "Level Trigger");

    for (round = 1; round <= 6; round++) {
        n = epoll_wait(epfd, &out, 1, 1000);            /* 최대 1 초 기다린다 */
        if (n == 0) {
            pending = 0;
            ioctl(pfd[0], FIONREAD, &pending);          /* Pipe 에 남아 있는 Byte 수 */
            printf("epoll_wait #%d: Timeout — Event 없음  (Pipe 에 남은 Data: %d Byte)\n", round, pending);
            continue;
        }
        got = read(pfd[0], buf, 3);                     /* 일부러 3 Byte 만 읽는다 */
        buf[got > 0 ? got : 0] = '\0';
        printf("epoll_wait #%d: EPOLLIN → read 3 Byte: \"%s\"\n", round, buf);
    }
    return EXIT_SUCCESS;
}
