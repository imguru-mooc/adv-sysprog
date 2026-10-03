/* select_demo.c - select 로 두 개의 Pipe 를 동시에 기다린다 */
#include <sys/select.h>
#include <sys/wait.h>
#include "producers.h"

int main(void)
{
    char    buf[128];
    int     fds[2], open_cnt = 2, i, calls = 0;
    double  t0 = now_s();
    ssize_t n;

    fds[0] = spawn_producer("fast", 200, 500, 4);
    fds[1] = spawn_producer("slow", 1200, 0, 1);

    while (open_cnt > 0) {
        fd_set rset;
        int    maxfd = -1, nready;

        FD_ZERO(&rset);                                 /* 1. 매번 처음부터 다시 만든다 */
        for (i = 0; i < 2; i++)
            if (fds[i] >= 0) {
                FD_SET(fds[i], &rset);
                if (fds[i] > maxfd)
                    maxfd = fds[i];
            }

        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);    /* 2. 전체를 Kernel 에 넘긴다 */
        calls++;
        if (nready < 0) {
            perror("select");
            return EXIT_FAILURE;
        }

        for (i = 0; i < 2; i++) {                       /* 3. 누가 준비됐는지 전부 검사한다 */
            if (fds[i] < 0 || !FD_ISSET(fds[i], &rset))
                continue;
            n = read(fds[i], buf, sizeof(buf) - 1);
            if (n > 0) {
                buf[n] = '\0';
                printf("[%4.1fs] fd %d: %s\n", now_s() - t0, fds[i], buf);
            } else {                                    /* 0 = EOF */
                printf("[%4.1fs] fd %d: EOF\n", now_s() - t0, fds[i]);
                close(fds[i]);
                fds[i] = -1;
                open_cnt--;
            }
        }
    }
    printf("select() 호출 %d 회\n", calls);
    while (wait(NULL) > 0)
        ;
    return EXIT_SUCCESS;
}
