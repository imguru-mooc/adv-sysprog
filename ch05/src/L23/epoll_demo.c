/* epoll_demo.c - 같은 일을 epoll 로 한다: 등록은 한 번, 기다릴 때는 "준비된 것만" 돌려받는다 */
#include <sys/epoll.h>
#include <sys/wait.h>
#include "producers.h"

#define MAX_EVENTS 8

int main(void)
{
    struct epoll_event ev, events[MAX_EVENTS];
    char               buf[128];
    int                epfd, fds[2], open_cnt = 2, i, calls = 0;
    double             t0 = now_s();
    ssize_t            n;

    fds[0] = spawn_producer("fast", 200, 500, 4);
    fds[1] = spawn_producer("slow", 1200, 0, 1);

    epfd = epoll_create1(EPOLL_CLOEXEC);                /* 1. epoll Instance 생성 (이것도 fd 다) */
    if (epfd < 0) {
        perror("epoll_create1");
        return EXIT_FAILURE;
    }
    for (i = 0; i < 2; i++) {                           /* 2. Interest List 에 등록: 한 번만 */
        ev.events  = EPOLLIN;
        ev.data.fd = fds[i];                            /*    Event 와 함께 돌려받을 값 */
        if (epoll_ctl(epfd, EPOLL_CTL_ADD, fds[i], &ev) < 0) {
            perror("epoll_ctl");
            return EXIT_FAILURE;
        }
    }

    while (open_cnt > 0) {
        int nready = epoll_wait(epfd, events, MAX_EVENTS, -1);  /* 3. 인자에 fd 목록이 없다 */

        calls++;
        if (nready < 0) {
            perror("epoll_wait");
            return EXIT_FAILURE;
        }
        for (i = 0; i < nready; i++) {                  /* 4. 준비된 것만 들어 있다. 검색이 필요 없다 */
            int fd = events[i].data.fd;

            n = read(fd, buf, sizeof(buf) - 1);
            if (n > 0) {
                buf[n] = '\0';
                printf("[%4.1fs] fd %d: %s   (events=0x%x)\n", now_s() - t0, fd, buf, events[i].events);
            } else {
                printf("[%4.1fs] fd %d: EOF      (events=0x%x%s)\n", now_s() - t0, fd, events[i].events,
                       (events[i].events & EPOLLHUP) ? " EPOLLHUP" : "");
                epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);       /* close 하면 자동 제거되지만 명시적으로 */
                close(fd);
                open_cnt--;
            }
        }
    }
    printf("epoll_wait() 호출 %d 회\n", calls);
    close(epfd);
    while (wait(NULL) > 0)
        ;
    return EXIT_SUCCESS;
}
