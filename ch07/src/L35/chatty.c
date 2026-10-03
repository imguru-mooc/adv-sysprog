/* chatty.c - "느린데 CPU 는 놀고 있다": 20 만 개의 Record 를 세 가지 방법으로 쓴다
 * 사용법: ./chatty slow     Record 마다 write()
 *         ./chatty fast     stdio Buffer (기본 4KB~)
 *         ./chatty missing  없는 설정 File 을 찾아 헤맨다 (strace 로 찾기)
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define N 200000

static void load_config(void)
{
    const char *paths[] = { "./chatty.conf", "/etc/chatty/chatty.conf", "/usr/local/etc/chatty.conf" };
    size_t      i;

    for (i = 0; i < sizeof(paths) / sizeof(paths[0]); i++) {
        int fd = open(paths[i], O_RDONLY);

        if (fd >= 0) {
            close(fd);
            return;
        }
    }
    fprintf(stderr, "오류: 설정을 불러올 수 없습니다\n");      /* 어느 File 인지 알려 주지 않는 불친절한 Message */
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
    const char *mode = argc > 1 ? argv[1] : "slow";
    char        line[32];
    int         i, len;

    if (strcmp(mode, "missing") == 0)
        load_config();

    if (strcmp(mode, "fast") == 0) {
        FILE *fp = fopen("chatty.out", "w");

        if (fp == NULL)
            return EXIT_FAILURE;
        for (i = 0; i < N; i++)
            fprintf(fp, "record %d\n", i);              /* User Space Buffer 에 모았다가 한꺼번에 write */
        fclose(fp);
    } else {
        int fd = open("chatty.out", O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (fd < 0)
            return EXIT_FAILURE;
        for (i = 0; i < N; i++) {
            len = snprintf(line, sizeof(line), "record %d\n", i);
            if (write(fd, line, (size_t)len) < 0)       /* Record 마다 System Call */
                return EXIT_FAILURE;
        }
        close(fd);
    }
    return EXIT_SUCCESS;
}
