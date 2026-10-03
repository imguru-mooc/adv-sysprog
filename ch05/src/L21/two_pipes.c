/* two_pipes.c - Blocking read 로 두 개의 입력을 동시에 기다릴 수 없다
 *   fast: 0.2 초마다 Message 를 보낸다      slow: 3 초 뒤에 한 번 보낸다
 * slow 를 먼저 read() 하면, fast 의 Message 가 쌓여 있어도 3 초 동안 아무것도 처리하지 못한다
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static double t0;

static double now(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static void producer(int fd, const char *name, int first_ms, int period_ms, int count)
{
    char msg[64];
    int  i;

    usleep((useconds_t)first_ms * 1000);
    for (i = 1; i <= count; i++) {
        int len = snprintf(msg, sizeof(msg), "%s#%d ", name, i);

        if (write(fd, msg, (size_t)len) < 0)
            _exit(EXIT_FAILURE);
        usleep((useconds_t)period_ms * 1000);
    }
    _exit(EXIT_SUCCESS);
}

int main(void)
{
    char    buf[256];
    int     fast[2], slow[2], i;
    ssize_t n;

    if (pipe(fast) < 0 || pipe(slow) < 0)
        return EXIT_FAILURE;
    if (fork() == 0) { close(fast[0]); producer(fast[1], "fast", 200, 200, 10); }
    if (fork() == 0) { close(slow[0]); producer(slow[1], "slow", 3000, 0, 1); }
    close(fast[1]);
    close(slow[1]);
    t0 = now();

    for (i = 0; i < 2; i++) {
        n = read(slow[0], buf, sizeof(buf) - 1);        /* slow 를 먼저 기다린다 */
        if (n > 0) { buf[n] = '\0'; printf("[%4.1fs] slow: %s\n", now() - t0, buf); }
        n = read(fast[0], buf, sizeof(buf) - 1);
        if (n > 0) { buf[n] = '\0'; printf("[%4.1fs] fast: %s\n", now() - t0, buf); }
        if (n == 0)
            break;
    }
    while (wait(NULL) > 0)
        ;
    return EXIT_SUCCESS;
}
