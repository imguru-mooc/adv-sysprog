/* block_read.c - 읽을 Data 가 없는 Pipe 에 read() 하면 Data 가 올 때까지 잠든다 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static double now(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

int main(void)
{
    char    buf[64];
    int     pfd[2];
    ssize_t n;
    double  t0;

    if (pipe(pfd) < 0) {
        perror("pipe");
        return EXIT_FAILURE;
    }
    if (fork() == 0) {                      /* Child: 2 초 뒤에 쓴다 */
        close(pfd[0]);
        sleep(2);
        if (write(pfd[1], "hello", 5) != 5)
            _exit(EXIT_FAILURE);
        _exit(EXIT_SUCCESS);
    }
    close(pfd[1]);

    printf("read() 호출... (ps 로 보면 이 Process 는 S 상태)\n");
    t0 = now();
    n = read(pfd[0], buf, sizeof(buf));     /* 여기서 Sleep */
    printf("read() = %zd, %.2f 초 동안 Block 되었습니다\n", n, now() - t0);

    wait(NULL);
    return EXIT_SUCCESS;
}
