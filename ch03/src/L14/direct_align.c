/* direct_align.c - O_DIRECT 의 제약: Buffer 주소, 크기, File Offset 이 모두 Block 크기에 정렬되어야 한다 */
#include "cachelib.h"
#include <errno.h>

#define NAME "align.dat"

static void try_read(int fd, void *buf, size_t len, off_t off, const char *tag)
{
    ssize_t n = pread(fd, buf, len, off);

    if (n < 0)
        printf("%-34s → 실패: %s\n", tag, strerror(errno));
    else
        printf("%-34s → %zd Byte\n", tag, n);
}

int main(void)
{
    char *raw, *aligned;
    long  total;
    int   fd;

    if (make_file(NAME, 1) < 0) {
        perror("make_file");
        return EXIT_FAILURE;
    }
    fd = open(NAME, O_RDONLY | O_DIRECT);
    if (fd < 0) {
        perror("open(O_DIRECT)");               /* tmpfs 등은 O_DIRECT 를 지원하지 않는다 */
        return EXIT_FAILURE;
    }
    evict(fd);
    if (posix_memalign((void **)&aligned, 4096, 8192) != 0)
        return EXIT_FAILURE;
    raw = aligned + 1;                          /* 일부러 어긋난 주소 */

    try_read(fd, aligned, 4096, 0,    "정렬된 Buffer, 4096 B, offset 0");
    try_read(fd, raw,     4096, 0,    "어긋난 Buffer 주소");
    try_read(fd, aligned, 1000, 0,    "어긋난 크기 (1000 B)");
    try_read(fd, aligned, 4096, 100,  "어긋난 offset (100)");

    printf("읽은 뒤 Page Cache 상주: %ld pages  (O_DIRECT 는 Cache 를 채우지 않는다)\n",
           resident_pages(fd, &total));
    free(aligned);
    close(fd);
    return EXIT_SUCCESS;
}
