/* epoll_inspect.c - epoll Instance 의 Interest List 를 /proc/self/fdinfo 로 들여다본다 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <unistd.h>

static void dump(int epfd, const char *tag)
{
    char  path[64], line[256];
    FILE *fp;

    snprintf(path, sizeof(path), "/proc/self/fdinfo/%d", epfd);
    fp = fopen(path, "r");
    if (fp == NULL)
        return;
    printf("== %s  (%s)\n", tag, path);
    while (fgets(line, sizeof(line), fp) != NULL)
        printf("   %s", line);
    fclose(fp);
}

int main(void)
{
    struct epoll_event ev;
    int                pfd[2], efd, tfd, epfd;

    if (pipe(pfd) < 0)
        return EXIT_FAILURE;
    efd  = eventfd(0, 0);
    tfd  = timerfd_create(CLOCK_MONOTONIC, 0);
    epfd = epoll_create1(0);
    printf("pipe(read)=%d  eventfd=%d  timerfd=%d  epoll=%d\n", pfd[0], efd, tfd, epfd);

    dump(epfd, "등록 전");

    ev.events = EPOLLIN;                 ev.data.fd = pfd[0]; epoll_ctl(epfd, EPOLL_CTL_ADD, pfd[0], &ev);
    ev.events = EPOLLIN | EPOLLET;       ev.data.fd = efd;    epoll_ctl(epfd, EPOLL_CTL_ADD, efd, &ev);
    ev.events = EPOLLIN | EPOLLONESHOT;  ev.data.fd = tfd;    epoll_ctl(epfd, EPOLL_CTL_ADD, tfd, &ev);
    dump(epfd, "3 개 등록 후");

    close(efd);                          /* close 하면 Interest List 에서 자동으로 빠진다 */
    dump(epfd, "eventfd 를 close 한 후");
    return EXIT_SUCCESS;
}
