/* sync_cost.c - 4KB Record 를 N 번 추가하면서 영속성 보장 수준을 바꿔 본다
 *   none      : write() 만
 *   fdatasync : 매 Record 마다 fdatasync()
 *   fsync     : 매 Record 마다 fsync()
 *   O_DSYNC   : open 할 때 O_DSYNC (write 가 곧 fdatasync)
 *   batch     : 100 Record 마다 한 번 fdatasync()  (Group Commit)
 * 사용법: ./sync_cost [record 수=1000]
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static char rec[4096];

static double now_ms(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e3 + (double)ts.tv_nsec / 1e6;
}

static void run(const char *tag, int oflag, int mode, int n)
{
    double t0, t1;
    int    i, fd = open("log.dat", O_WRONLY | O_CREAT | O_TRUNC | oflag, 0644);

    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    t0 = now_ms();
    for (i = 0; i < n; i++) {
        if (write(fd, rec, sizeof(rec)) != (ssize_t)sizeof(rec)) {
            perror("write");
            exit(EXIT_FAILURE);
        }
        if (mode == 1 || (mode == 3 && i % 100 == 99))
            fdatasync(fd);
        else if (mode == 2)
            fsync(fd);
    }
    if (mode == 3)
        fdatasync(fd);
    t1 = now_ms();
    close(fd);
    printf("%-22s %9.1f ms   %10.1f records/s\n", tag, t1 - t0, n / ((t1 - t0) / 1e3));
}

int main(int argc, char *argv[])
{
    int n = argc > 1 ? atoi(argv[1]) : 1000;

    if (n < 1)
        return EXIT_FAILURE;
    memset(rec, 'r', sizeof(rec));
    printf("4KB Record × %d\n", n);
    run("none (write 만)", 0, 0, n);
    run("fdatasync 매번", 0, 1, n);
    run("fsync 매번", 0, 2, n);
    run("O_DSYNC", O_DSYNC, 0, n);
    run("fdatasync 100 개마다", 0, 3, n);
    unlink("log.dat");
    return EXIT_SUCCESS;
}
