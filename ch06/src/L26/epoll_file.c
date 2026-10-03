/* epoll_file.c - 일반 File 은 epoll 에 등록할 수 없고, poll 은 "항상 준비됨" 이라고 답한다 */
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

int main(void)
{
    struct epoll_event ev = { .events = EPOLLIN };
    struct pollfd      p;
    int                fd = open("/etc/passwd", O_RDONLY | O_NONBLOCK);
    int                epfd = epoll_create1(0), r;

    if (fd < 0 || epfd < 0)
        return EXIT_FAILURE;

    ev.data.fd = fd;
    r = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    printf("epoll_ctl(ADD, 일반 File) = %d, errno = %d (%s)\n", r, errno, strerror(errno));

    p.fd = fd;
    p.events = POLLIN | POLLOUT;
    r = poll(&p, 1, 0);
    printf("poll(일반 File)           = %d, revents = 0x%x  (POLLIN%s) → 언제나 \"준비됨\"\n", r,
           (unsigned)p.revents, (p.revents & POLLOUT) ? " | POLLOUT" : "");
    printf("→ Readiness 모델은 Disk File 에 대해 아무 정보도 주지 못한다.\n");
    printf("  O_NONBLOCK 으로 열었어도 Page Cache Miss 면 read() 는 Disk 를 기다리며 잠든다.\n");
    return EXIT_SUCCESS;
}
