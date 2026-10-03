/* readahead.c - 1 Byte 만 읽었는데 Page Cache 에는 몇 Page 가 올라오는가
 * 순차 읽기와 무작위 읽기에서 Kernel 의 Readahead 가 어떻게 달라지는지 본다
 */
#include "cachelib.h"

#define NAME "ra.dat"

static void report(int fd, const char *tag)
{
    long total, res = resident_pages(fd, &total);

    printf("%-44s cache %6ld pages (%ld KB)\n", tag, res, res * 4);
}

int main(void)
{
    char  c, buf[4096];
    off_t off;
    int   i, fd;

    if (access(NAME, F_OK) != 0 && make_file(NAME, 64) < 0) {
        perror("make_file");
        return EXIT_FAILURE;
    }
    fd = open(NAME, O_RDONLY);
    if (fd < 0) {
        perror(NAME);
        return EXIT_FAILURE;
    }

    evict(fd);
    report(fd, "시작 (Cache 비움)");
    if (pread(fd, &c, 1, 0) != 1)
        return EXIT_FAILURE;
    report(fd, "1 Byte 읽음 (offset 0)");

    evict(fd);
    lseek(fd, 0, SEEK_SET);
    for (i = 0; i < 64; i++)                        /* 순차: 4KB × 64 = 256KB */
        if (read(fd, buf, sizeof(buf)) != (ssize_t)sizeof(buf))
            return EXIT_FAILURE;
    report(fd, "순차로 256KB 읽음");

    evict(fd);
    posix_fadvise(fd, 0, 0, POSIX_FADV_RANDOM);     /* "무작위로 읽을 것" 이라고 알려 준다 */
    for (i = 0; i < 64; i++) {
        off = (off_t)((i * 7919L) % 16384) * 4096;
        if (pread(fd, buf, sizeof(buf), off) != (ssize_t)sizeof(buf))
            return EXIT_FAILURE;
    }
    report(fd, "FADV_RANDOM 후 무작위로 4KB × 64 읽음");

    close(fd);
    return EXIT_SUCCESS;
}
