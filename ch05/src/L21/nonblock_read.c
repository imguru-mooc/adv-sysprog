/* nonblock_read.c - O_NONBLOCK: 읽을 Data 가 없으면 기다리지 않고 EAGAIN 으로 즉시 return 한다
 * 그 결과 "될 때까지 계속 물어보는" Busy Polling 이 되면 CPU 를 낭비한다
 */
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    struct rusage ru;
    char          buf[64];
    long          tries = 0;
    int           pfd[2], flags;
    ssize_t       n;

    if (pipe(pfd) < 0) {
        perror("pipe");
        return EXIT_FAILURE;
    }
    if (fork() == 0) {
        close(pfd[0]);
        sleep(2);
        if (write(pfd[1], "hello", 5) != 5)
            _exit(EXIT_FAILURE);
        _exit(EXIT_SUCCESS);
    }
    close(pfd[1]);

    flags = fcntl(pfd[0], F_GETFL);                     /* 기존 Flag 를 읽어서 */
    if (fcntl(pfd[0], F_SETFL, flags | O_NONBLOCK) < 0) /* O_NONBLOCK 을 더한다 */
        perror("fcntl");

    n = read(pfd[0], buf, sizeof(buf));
    printf("첫 read() = %zd, errno = %d (%s)\n", n, errno, strerror(errno));

    for (;;) {                                          /* Busy Polling */
        n = read(pfd[0], buf, sizeof(buf));
        tries++;
        if (n >= 0)
            break;
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("read");
            return EXIT_FAILURE;
        }
    }
    getrusage(RUSAGE_SELF, &ru);
    printf("read() 를 %ld 번 호출한 끝에 %zd Byte 를 읽었습니다\n", tries, n);
    printf("그동안 사용한 CPU 시간: user %.2f 초 + system %.2f 초\n",
           (double)ru.ru_utime.tv_sec + (double)ru.ru_utime.tv_usec / 1e6,
           (double)ru.ru_stime.tv_sec + (double)ru.ru_stime.tv_usec / 1e6);
    wait(NULL);
    return EXIT_SUCCESS;
}
