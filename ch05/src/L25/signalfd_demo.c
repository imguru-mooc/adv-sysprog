/* signalfd_demo.c - Signal 을 Handler 가 아니라 fd 로 받는다: 일반 코드처럼 순서대로 처리할 수 있다
 * 실행 후 Ctrl+C 를 누르거나 다른 Terminal 에서 kill -TERM <pid>, kill -USR1 <pid>
 */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <unistd.h>

int main(void)
{
    struct signalfd_siginfo si;
    struct epoll_event      ev, out;
    sigset_t                mask;
    int                     sfd, epfd, running = 1;

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &mask, NULL) < 0)        /* 1. 먼저 Block 한다: 기본 동작(종료)을 막는다 */
        return EXIT_FAILURE;

    sfd  = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);     /* 2. Block 된 Signal 을 fd 로 받는다 */
    epfd = epoll_create1(EPOLL_CLOEXEC);
    if (sfd < 0 || epfd < 0)
        return EXIT_FAILURE;
    ev.events  = EPOLLIN;
    ev.data.fd = sfd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, sfd, &ev);

    printf("pid %d: SIGINT(Ctrl+C), SIGTERM, SIGUSR1 을 기다립니다\n", (int)getpid());
    while (running) {
        if (epoll_wait(epfd, &out, 1, -1) < 1)
            continue;
        while (read(sfd, &si, sizeof(si)) == (ssize_t)sizeof(si)) {     /* 3. Signal 하나 = 구조체 하나 */
            printf("Signal %u (%s), 보낸 pid %u\n", si.ssi_signo, strsignal((int)si.ssi_signo), si.ssi_pid);
            if (si.ssi_signo == SIGUSR1) {
                printf("   → 설정을 다시 읽습니다 (printf, malloc 등을 마음대로 써도 됩니다)\n");
            } else {
                printf("   → 정리 후 종료합니다 (Graceful Shutdown)\n");
                running = 0;
            }
        }
    }
    close(sfd);
    close(epfd);
    return EXIT_SUCCESS;
}
