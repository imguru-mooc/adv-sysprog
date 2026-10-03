/* poll_demo.c - 같은 일을 poll 로 한다: Bitmask 대신 구조체 배열, 입력과 출력이 분리된다 */
#include <poll.h>
#include <sys/wait.h>
#include "producers.h"

int main(void)
{
    struct pollfd pfds[2];
    char          buf[128];
    int           open_cnt = 2, i, calls = 0;
    double        t0 = now_s();
    ssize_t       n;

    pfds[0].fd = spawn_producer("fast", 200, 500, 4);
    pfds[1].fd = spawn_producer("slow", 1200, 0, 1);
    pfds[0].events = pfds[1].events = POLLIN;           /* 관심 Event 는 한 번만 설정 */

    while (open_cnt > 0) {
        int nready = poll(pfds, 2, -1);                 /* 그래도 배열 전체가 매번 Kernel 로 복사된다 */

        calls++;
        if (nready < 0) {
            perror("poll");
            return EXIT_FAILURE;
        }
        for (i = 0; i < 2; i++) {                       /* 결과는 revents 에. 여전히 전부 검사 */
            if (pfds[i].fd < 0 || pfds[i].revents == 0)
                continue;
            n = read(pfds[i].fd, buf, sizeof(buf) - 1);
            if (n > 0) {
                buf[n] = '\0';
                printf("[%4.1fs] fd %d: %s   (revents=0x%x)\n", now_s() - t0, pfds[i].fd, buf,
                       (unsigned)pfds[i].revents);
            } else {
                printf("[%4.1fs] fd %d: EOF      (revents=0x%x%s)\n", now_s() - t0, pfds[i].fd,
                       (unsigned)pfds[i].revents, (pfds[i].revents & POLLHUP) ? " POLLHUP" : "");
                close(pfds[i].fd);
                pfds[i].fd = -1;                        /* 음수 fd 는 poll 이 무시한다 */
                open_cnt--;
            }
        }
    }
    printf("poll() 호출 %d 회\n", calls);
    while (wait(NULL) > 0)
        ;
    return EXIT_SUCCESS;
}
