/* short_io.c - read()/write() 는 요청한 만큼 처리한다는 보장이 없다. 올바른 Loop 작성법
 * 사용법: (sleep 1; echo hello; sleep 1; echo world) | ./short_io
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* 요청한 count 를 모두 쓸 때까지 반복한다 */
static ssize_t write_all(int fd, const void *buf, size_t count)
{
    const char *p    = buf;
    size_t      left = count;

    while (left > 0) {
        ssize_t n = write(fd, p, left);

        if (n < 0) {
            if (errno == EINTR)             /* Signal 로 중단됨: 다시 시도 */
                continue;
            return -1;
        }
        p    += n;
        left -= (size_t)n;
    }
    return (ssize_t)count;
}

int main(void)
{
    char    buf[4096];
    ssize_t n;
    int     calls = 0;

    /* 4096 Byte 를 요청하지만 Pipe 에는 그때그때 도착한 만큼만 들어 있다 */
    while ((n = read(STDIN_FILENO, buf, sizeof(buf))) != 0) {
        if (n < 0) {
            if (errno == EINTR)
                continue;
            perror("read");
            return EXIT_FAILURE;
        }
        calls++;
        fprintf(stderr, "read() #%d: 4096 요청 → %zd Byte\n", calls, n);
        if (write_all(STDOUT_FILENO, buf, (size_t)n) < 0) {
            perror("write");
            return EXIT_FAILURE;
        }
    }
    fprintf(stderr, "read() = 0 → EOF\n");
    return EXIT_SUCCESS;
}
