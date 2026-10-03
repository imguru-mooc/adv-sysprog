/* posix_aio.c - POSIX AIO(aio_read) 는 비동기처럼 보이지만, glibc 가 몰래 만든 Thread 가 Blocking read 를 대신 한다 */
#include <aio.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int count_threads(void)
{
    struct dirent *de;
    DIR           *d = opendir("/proc/self/task");
    int            n = 0;

    if (d == NULL)
        return -1;
    while ((de = readdir(d)) != NULL)
        if (de->d_name[0] != '.')
            n++;
    closedir(d);
    return n;
}

int main(void)
{
    static char  buf[4][4096];
    struct aiocb cb[4];
    int          i, fd = open("/etc/passwd", O_RDONLY);

    if (fd < 0)
        return EXIT_FAILURE;
    printf("aio_read 호출 전 Thread 수: %d\n", count_threads());

    memset(cb, 0, sizeof(cb));
    for (i = 0; i < 4; i++) {
        cb[i].aio_fildes = fd;
        cb[i].aio_buf    = buf[i];
        cb[i].aio_nbytes = sizeof(buf[i]);
        cb[i].aio_offset = 0;
        if (aio_read(&cb[i]) < 0) {                     /* 요청만 하고 즉시 return */
            perror("aio_read");
            return EXIT_FAILURE;
        }
    }
    printf("aio_read 4 개 요청 직후 Thread 수: %d   ← glibc 가 Worker Thread 를 만들었다\n", count_threads());

    for (i = 0; i < 4; i++) {
        const struct aiocb *list[1] = { &cb[i] };

        while (aio_error(&cb[i]) == EINPROGRESS)
            aio_suspend(list, 1, NULL);                 /* 완료 대기 */
        printf("요청 %d 완료: %zd Byte\n", i, aio_return(&cb[i]));
    }
    close(fd);
    return EXIT_SUCCESS;
}
