/* offcpu.c - "느린데 perf 에는 아무것도 안 보인다": 시간의 대부분을 CPU 밖에서(Sleep, I/O 대기) 보내는 프로그램 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

__attribute__((noinline)) static void wait_for_backend(void)
{
    usleep(20 * 1000);                                  /* 외부 Service 의 응답을 기다리는 상황 */
}

__attribute__((noinline)) static void save_record(int fd, int i)
{
    char line[64];
    int  len = snprintf(line, sizeof(line), "record %d\n", i);

    if (write(fd, line, (size_t)len) < 0)
        exit(EXIT_FAILURE);
    fsync(fd);                                          /* 매번 fsync (L15) */
}

__attribute__((noinline)) static unsigned long compute(int i)
{
    unsigned long h = (unsigned long)i;
    int           k;

    for (k = 0; k < 200000; k++)
        h = h * 6364136223846793005UL + 1442695040888963407UL;
    return h;
}

int main(void)
{
    unsigned long acc = 0;
    int           i, fd = open("offcpu.dat", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd < 0)
        return EXIT_FAILURE;
    for (i = 0; i < 100; i++) {
        acc += compute(i);
        wait_for_backend();
        save_record(fd, i);
    }
    printf("result = %lu\n", acc);
    close(fd);
    return EXIT_SUCCESS;
}
