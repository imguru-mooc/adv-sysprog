/* dirty_watch.c - write() 가 끝난 뒤 Data 는 어디에 있는가: /proc/meminfo 의 Dirty 를 관찰한다 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define NAME "dirty.dat"
#define MB   200

static char buf[1 << 20];

static double now_ms(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e3 + (double)ts.tv_nsec / 1e6;
}

static void show(const char *tag)
{
    char  line[128];
    long  dirty = 0, wb = 0;
    FILE *fp = fopen("/proc/meminfo", "r");

    if (fp == NULL)
        return;
    while (fgets(line, sizeof(line), fp) != NULL) {
        sscanf(line, "Dirty: %ld kB", &dirty);
        sscanf(line, "Writeback: %ld kB", &wb);
    }
    fclose(fp);
    printf("%-28s Dirty %8ld kB   Writeback %8ld kB\n", tag, dirty, wb);
}

int main(void)
{
    double t0, t1, t2;
    int    i, fd = open(NAME, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd < 0) {
        perror(NAME);
        return EXIT_FAILURE;
    }
    memset(buf, 'd', sizeof(buf));
    sync();                                     /* 기존 Dirty Page 를 먼저 비워 관찰을 깨끗하게 */
    show("시작");

    t0 = now_ms();
    for (i = 0; i < MB; i++)
        if (write(fd, buf, sizeof(buf)) != (ssize_t)sizeof(buf)) {
            perror("write");
            return EXIT_FAILURE;
        }
    t1 = now_ms();
    show("write() 200MB 완료 직후");
    printf("   write() 에 걸린 시간 : %8.1f ms  (%.0f MB/s)\n", t1 - t0, MB / ((t1 - t0) / 1e3));

    if (fsync(fd) < 0)
        perror("fsync");
    t2 = now_ms();
    show("fsync() 완료 직후");
    printf("   fsync() 에 걸린 시간 : %8.1f ms\n", t2 - t1);

    close(fd);
    return EXIT_SUCCESS;
}
