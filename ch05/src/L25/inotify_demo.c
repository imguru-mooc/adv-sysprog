/* inotify_demo.c - Directory 의 변화를 fd 로 받는다
 * 실행 후 다른 Terminal 에서:  touch watched/a; echo hi >> watched/a; mv watched/a watched/b; rm watched/b
 * 사용법: ./inotify_demo [받을 Event 수=7]
 */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <unistd.h>

static const char *name_of(uint32_t mask)
{
    if (mask & IN_CREATE)      return "CREATE";
    if (mask & IN_MODIFY)      return "MODIFY";
    if (mask & IN_CLOSE_WRITE) return "CLOSE_WRITE";
    if (mask & IN_MOVED_FROM)  return "MOVED_FROM";
    if (mask & IN_MOVED_TO)    return "MOVED_TO";
    if (mask & IN_DELETE)      return "DELETE";
    return "?";
}

int main(int argc, char *argv[])
{
    char               buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
    struct epoll_event ev, out;
    int                ifd, epfd, left = argc > 1 ? atoi(argv[1]) : 7;
    ssize_t            len;

    mkdir("watched", 0755);
    ifd  = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    epfd = epoll_create1(EPOLL_CLOEXEC);
    if (ifd < 0 || epfd < 0 ||
        inotify_add_watch(ifd, "watched", IN_CREATE | IN_MODIFY | IN_CLOSE_WRITE | IN_MOVE | IN_DELETE) < 0) {
        perror("inotify");
        return EXIT_FAILURE;
    }
    ev.events  = EPOLLIN;
    ev.data.fd = ifd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, ifd, &ev);
    printf("watched/ 를 감시합니다 (Event %d 개를 받으면 종료)\n", left);

    while (left > 0) {
        if (epoll_wait(epfd, &out, 1, -1) < 1)
            continue;
        while ((len = read(ifd, buf, sizeof(buf))) > 0) {       /* 한 번의 read 에 Event 여러 개가 올 수 있다 */
            char *p = buf;

            while (p < buf + len) {
                struct inotify_event *e = (struct inotify_event *)p;

                printf("%-12s %s\n", name_of(e->mask), e->len ? e->name : "");
                left--;
                p += sizeof(*e) + e->len;                       /* 가변 길이 */
            }
        }
    }
    return EXIT_SUCCESS;
}
