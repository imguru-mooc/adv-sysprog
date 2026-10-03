/* spawn.c - Child 를 계속 만든다 (각 Child 는 잠만 잔다). pids.max 에 닿으면 fork 가 실패한다
 * 사용법: ./spawn [최대 개수=50]
 */
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    static pid_t kids[1024];
    int          max = argc > 1 ? atoi(argv[1]) : 50, n, i;

    if (max > 1024)
        max = 1024;
    for (n = 1; n <= max; n++) {
        pid_t pid = fork();

        if (pid < 0) {
            printf("fork #%d 실패: %s  ← pids.max\n", n, strerror(errno));
            break;
        }
        if (pid == 0) {
            pause();
            _exit(0);
        }
        kids[n - 1] = pid;
    }
    printf("만들어진 Child: %d 개\n", n - 1);
    for (i = 0; i < n - 1; i++)                          /* 만든 Child 들을 정리 */
        kill(kids[i], SIGTERM);
    while (wait(NULL) > 0)
        ;
    return EXIT_SUCCESS;
}
