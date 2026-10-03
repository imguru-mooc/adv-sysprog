/* bufsize.c - 같은 File 을 Buffer 크기만 바꿔 가며 read() 한다. System Call 횟수와 시간의 관계
 * 사용법: ./bufsize make     (64MB File data.dat 생성)
 *         ./bufsize          (1B ~ 1MB Buffer 로 읽기)
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define FILE_NAME "data.dat"
#define FILE_SIZE (64UL * 1024 * 1024)

static char buf[1 << 20];

static int make_file(void)
{
    size_t done;
    int    fd = open(FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd < 0) {
        perror("open");
        return EXIT_FAILURE;
    }
    memset(buf, 'x', sizeof(buf));
    for (done = 0; done < FILE_SIZE; done += sizeof(buf))
        if (write(fd, buf, sizeof(buf)) != (ssize_t)sizeof(buf)) {
            perror("write");
            return EXIT_FAILURE;
        }
    close(fd);
    printf("%s (%lu MB) 생성\n", FILE_NAME, FILE_SIZE >> 20);
    return EXIT_SUCCESS;
}

static void run(size_t bs, size_t limit)
{
    struct timespec t0, t1;
    size_t          total = 0;
    long            calls = 0;
    ssize_t         n;
    double          ms;
    int             fd = open(FILE_NAME, O_RDONLY);

    if (fd < 0) {
        perror(FILE_NAME " (먼저 ./bufsize make)");
        exit(EXIT_FAILURE);
    }
    clock_gettime(CLOCK_MONOTONIC, &t0);
    while (total < limit && (n = read(fd, buf, bs)) > 0) {
        total += (size_t)n;
        calls++;
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    close(fd);
    ms = (double)(t1.tv_sec - t0.tv_sec) * 1e3 + (double)(t1.tv_nsec - t0.tv_nsec) / 1e6;
    printf("%9zu B %10ld calls %9.1f ms %9.1f MB/s\n", bs, calls, ms, (double)total / 1048576.0 / (ms / 1e3));
}

int main(int argc, char *argv[])
{
    size_t sizes[] = { 1, 16, 256, 4096, 65536, 1048576 };
    size_t i;

    if (argc > 1 && strcmp(argv[1], "make") == 0)
        return make_file();

    printf("%11s %16s %12s %14s\n", "buffer", "read() 횟수", "시간", "처리량");
    for (i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++)
        run(sizes[i], sizes[i] == 1 ? FILE_SIZE / 16 : FILE_SIZE);   /* 1B 는 너무 느려서 4MB 만 */
    return EXIT_SUCCESS;
}
