/* et_rearm.c - Edge 는 "새 Data 가 도착하는 순간" 이다
 * 3 Byte 만 읽고 남겨 둔 상태에서 Writer 가 다시 쓰면 Event 가 또 발생한다
 */
#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <unistd.h>

static void step(int epfd, int rfd, const char *tag)
{
    struct epoll_event out;
    int                pending = 0, n = epoll_wait(epfd, &out, 1, 300);

    ioctl(rfd, FIONREAD, &pending);                     /* Pipe 에 쌓여 있는 Byte 수 */
    printf("%-34s epoll_wait = %d   (Pipe 에 %2d Byte)\n", tag, n, pending);
}

int main(void)
{
    struct epoll_event ev;
    char               buf[4];
    int                pfd[2], epfd;

    if (pipe2(pfd, O_NONBLOCK) < 0 || (epfd = epoll_create1(0)) < 0)
        return EXIT_FAILURE;
    ev.events  = EPOLLIN | EPOLLET;
    ev.data.fd = pfd[0];
    epoll_ctl(epfd, EPOLL_CTL_ADD, pfd[0], &ev);

    step(epfd, pfd[0], "1. 비어 있음");
    if (write(pfd[1], "HELLO LINUX", 11) != 11) return EXIT_FAILURE;
    step(epfd, pfd[0], "2. write 11 Byte  (없음 → 있음)");
    if (read(pfd[0], buf, 3) != 3) return EXIT_FAILURE;
    step(epfd, pfd[0], "3. 3 Byte 만 읽음  (변화 없음)");
    if (write(pfd[1], "!", 1) != 1) return EXIT_FAILURE;
    step(epfd, pfd[0], "4. write 1 Byte   (새 Data 도착)");
    step(epfd, pfd[0], "5. 아무 일도 없음");
    return EXIT_SUCCESS;
}
