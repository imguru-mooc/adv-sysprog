/* pipe_full.c - 쓰기에도 Blocking 이 있다: 아무도 읽지 않는 Pipe 를 가득 채운다 */
#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
    char    chunk[4096];
    long    total = 0;
    int     pfd[2];
    ssize_t n;

    if (pipe2(pfd, O_NONBLOCK) < 0) {       /* 만들 때부터 Non-blocking */
        perror("pipe2");
        return EXIT_FAILURE;
    }
    memset(chunk, 'x', sizeof(chunk));

    while ((n = write(pfd[1], chunk, sizeof(chunk))) > 0)
        total += n;

    printf("write 가 멈춘 시점: 누적 %ld Byte, errno = %d (%s)\n", total, errno, strerror(errno));
    printf("F_GETPIPE_SZ = %d\n", fcntl(pfd[1], F_GETPIPE_SZ));
    printf("Blocking Mode 였다면 이 지점에서 write() 가 영원히 잠들었을 것입니다.\n");
    return EXIT_SUCCESS;
}
