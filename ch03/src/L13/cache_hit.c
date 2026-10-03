/* cache_hit.c - 같은 File 을 세 번 읽는다: Cache 를 비운 직후, 바로 다시, 또 다시
 * 사용법: ./cache_hit [file]   (인자가 없으면 256MB 의 cache.dat 를 만든다)
 */
#include "cachelib.h"

static char buf[1 << 20];

static void read_all(const char *name, const char *tag)
{
    long    total, before, after;
    ssize_t n;
    double  t0, t1;
    size_t  bytes = 0;
    int     fd = open(name, O_RDONLY);

    if (fd < 0) {
        perror(name);
        exit(EXIT_FAILURE);
    }
    before = resident_pages(fd, &total);
    t0 = now_ms();
    while ((n = read(fd, buf, sizeof(buf))) > 0)
        bytes += (size_t)n;
    t1 = now_ms();
    after = resident_pages(fd, &total);
    printf("%-22s cache %6ld → %6ld / %ld pages   %8.1f ms   %8.0f MB/s\n", tag, before, after, total,
           t1 - t0, (double)bytes / 1048576.0 / ((t1 - t0) / 1e3));
    close(fd);
}

int main(int argc, char *argv[])
{
    const char *name = argc > 1 ? argv[1] : "cache.dat";
    int         fd;

    if (argc == 1 && access(name, F_OK) != 0) {
        printf("%s (256MB) 생성 중...\n", name);
        if (make_file(name, 256) < 0) {
            perror("make_file");
            return EXIT_FAILURE;
        }
    }
    fd = open(name, O_RDONLY);
    if (fd < 0) {
        perror(name);
        return EXIT_FAILURE;
    }
    evict(fd);                              /* 이 File 의 Cache 만 비운다 */
    close(fd);

    read_all(name, "1. Cache 비운 직후");
    read_all(name, "2. 바로 다시");
    read_all(name, "3. 또 다시");
    return EXIT_SUCCESS;
}
