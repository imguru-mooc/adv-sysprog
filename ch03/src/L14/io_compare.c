/* io_compare.c - 같은 256MB File 을 네 가지 방법으로 처음부터 끝까지 읽는다
 *   buffered 4KB / buffered 1MB / O_DIRECT 1MB / mmap
 * 각 방법을 Cold(Cache 비움) 와 Warm(바로 다시) 으로 두 번씩 측정한다
 */
#include "cachelib.h"
#include <sys/resource.h>

#define NAME "io.dat"
#define MB   256

static unsigned long checksum;

static void run_read(int flags, size_t bs)
{
    void   *buf;
    ssize_t n, i;
    int     fd = open(NAME, O_RDONLY | flags);

    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }
    if (posix_memalign(&buf, 4096, bs) != 0)        /* O_DIRECT 는 정렬된 Buffer 가 필요하다 */
        exit(EXIT_FAILURE);
    while ((n = read(fd, buf, bs)) > 0)
        for (i = 0; i < n; i += 64)                 /* 읽은 Data 를 실제로 사용한다 (Cache Line 마다 1 Byte) */
            checksum += ((unsigned char *)buf)[i];
    free(buf);
    close(fd);
}

static void run_mmap(void)
{
    struct stat    st;
    unsigned char *p;
    off_t          i;
    int            fd = open(NAME, O_RDONLY);

    if (fd < 0 || fstat(fd, &st) < 0)
        exit(EXIT_FAILURE);
    p = mmap(NULL, (size_t)st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (p == MAP_FAILED)
        exit(EXIT_FAILURE);
    madvise(p, (size_t)st.st_size, MADV_SEQUENTIAL);
    for (i = 0; i < st.st_size; i += 64)
        checksum += p[i];
    munmap(p, (size_t)st.st_size);
    close(fd);
}

static void measure(const char *tag, int mode)
{
    struct rusage r0, r1;
    double        t0, t1;
    int           pass, fd;

    for (pass = 0; pass < 2; pass++) {
        if (pass == 0) {
            fd = open(NAME, O_RDONLY);
            evict(fd);
            close(fd);
        }
        getrusage(RUSAGE_SELF, &r0);
        t0 = now_ms();
        switch (mode) {
        case 0: run_read(0, 4096);            break;
        case 1: run_read(0, 1 << 20);         break;
        case 2: run_read(O_DIRECT, 1 << 20);  break;
        default: run_mmap();                  break;
        }
        t1 = now_ms();
        getrusage(RUSAGE_SELF, &r1);
        printf("%-16s %-5s %8.1f ms %7.0f MB/s   minflt %6ld  majflt %5ld\n", tag, pass ? "warm" : "cold",
               t1 - t0, MB / ((t1 - t0) / 1e3), r1.ru_minflt - r0.ru_minflt, r1.ru_majflt - r0.ru_majflt);
    }
}

int main(void)
{
    if (access(NAME, F_OK) != 0) {
        printf("%s (%dMB) 생성 중...\n", NAME, MB);
        if (make_file(NAME, MB) < 0) {
            perror("make_file");
            return EXIT_FAILURE;
        }
    }
    measure("buffered 4KB", 0);
    measure("buffered 1MB", 1);
    measure("O_DIRECT 1MB", 2);
    measure("mmap", 3);
    printf("(checksum %lu)\n", checksum);
    return EXIT_SUCCESS;
}
