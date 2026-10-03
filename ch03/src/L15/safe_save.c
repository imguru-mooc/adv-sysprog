/* safe_save.c - 설정 File 을 "전부 옛 내용이거나, 전부 새 내용이거나" 로 안전하게 교체한다
 *   1. 임시 File 에 쓴다       2. fsync(임시 File)
 *   3. rename(임시 → 원본)     4. fsync(Directory)
 * 사용법: ./safe_save "새 내용"
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TARGET "config.txt"
#define TMP    "config.txt.tmp"

static int write_all(int fd, const char *p, size_t left)
{
    while (left > 0) {
        ssize_t n = write(fd, p, left);

        if (n < 0)
            return -1;
        p    += n;
        left -= (size_t)n;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    const char *text = argc > 1 ? argv[1] : "mode=safe";
    int         fd, dfd;

    fd = open(TMP, O_WRONLY | O_CREAT | O_TRUNC, 0644);             /* 1. 원본은 건드리지 않는다 */
    if (fd < 0 || write_all(fd, text, strlen(text)) < 0 || write_all(fd, "\n", 1) < 0) {
        perror(TMP);
        return EXIT_FAILURE;
    }
    if (fsync(fd) < 0) {                                            /* 2. 새 내용이 Disk 에 있음을 보장 */
        perror("fsync");                                            /*    실패하면 절대 rename 하지 않는다 */
        return EXIT_FAILURE;
    }
    close(fd);

    if (rename(TMP, TARGET) < 0) {                                  /* 3. 이름 교체는 Atomic */
        perror("rename");
        return EXIT_FAILURE;
    }

    dfd = open(".", O_RDONLY | O_DIRECTORY);                        /* 4. "이름이 바뀌었다" 는 사실도 Disk 에 */
    if (dfd < 0 || fsync(dfd) < 0) {
        perror("fsync(dir)");
        return EXIT_FAILURE;
    }
    close(dfd);

    printf("%s 저장 완료\n", TARGET);
    return EXIT_SUCCESS;
}
