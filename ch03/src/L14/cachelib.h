/* cachelib.h - Page Cache 관찰용 공통 함수 */
#ifndef CACHELIB_H
#define CACHELIB_H

#define _GNU_SOURCE
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

static inline double now_ms(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e3 + (double)ts.tv_nsec / 1e6;
}

/* File 의 Page 중 지금 Page Cache 에 올라와 있는 개수를 센다 (mmap + mincore) */
static inline long resident_pages(int fd, long *total)
{
    struct stat    st;
    unsigned char *vec;
    void          *map;
    long           i, n, res = 0;

    if (fstat(fd, &st) < 0 || st.st_size == 0)
        return -1;
    n   = (st.st_size + 4095) / 4096;
    map = mmap(NULL, (size_t)st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    vec = malloc((size_t)n);
    if (map == MAP_FAILED || vec == NULL)
        return -1;
    if (mincore(map, (size_t)st.st_size, vec) == 0)     /* 접근하지 않고 상주 여부만 묻는다 */
        for (i = 0; i < n; i++)
            res += vec[i] & 1;
    munmap(map, (size_t)st.st_size);
    free(vec);
    if (total != NULL)
        *total = n;
    return res;
}

/* 이 File 의 Page Cache 를 비운다. root 권한이 필요 없다 */
static inline void evict(int fd)
{
    fsync(fd);                                          /* Dirty Page 는 버릴 수 없으므로 먼저 기록 */
    posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);
}

static inline int make_file(const char *name, size_t mb)
{
    static char buf[1 << 20];
    size_t      i;
    int         fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd < 0)
        return -1;
    memset(buf, 'x', sizeof(buf));
    for (i = 0; i < mb; i++)
        if (write(fd, buf, sizeof(buf)) != (ssize_t)sizeof(buf))
            return -1;
    fsync(fd);
    close(fd);
    return 0;
}

#endif
