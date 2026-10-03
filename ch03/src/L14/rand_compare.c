/* rand_compare.c - Cache 에 올라와 있는 File 에서 작은 Record 를 무작위로 읽는다: pread vs mmap
 * (System Call + 복사) 대 (Memory 접근) 의 차이를 본다
 */
#include "cachelib.h"

#define NAME  "io.dat"
#define COUNT 2000000

int main(void)
{
    struct stat    st;
    unsigned char *p, rec[64];
    unsigned long  sum = 0;
    unsigned       seed = 12345;
    double         t0, t1, t2;
    off_t          off;
    long           i, nrec;
    int            fd = open(NAME, O_RDONLY);

    if (fd < 0 || fstat(fd, &st) < 0) {
        perror(NAME " (먼저 ./io_compare 실행)");
        return EXIT_FAILURE;
    }
    nrec = st.st_size / 64;
    p = mmap(NULL, (size_t)st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (p == MAP_FAILED)
        return EXIT_FAILURE;
    for (off = 0; off < st.st_size; off += 4096)    /* 준비: Cache 와 PTE 를 모두 채운다 */
        sum += p[off];

    t0 = now_ms();
    for (i = 0; i < COUNT; i++) {
        seed = seed * 1103515245u + 12345u;
        off  = (off_t)(seed % (unsigned long)nrec) * 64;
        if (pread(fd, rec, sizeof(rec), off) != (ssize_t)sizeof(rec))
            return EXIT_FAILURE;
        sum += rec[0];
    }
    t1 = now_ms();
    for (i = 0; i < COUNT; i++) {
        seed = seed * 1103515245u + 12345u;
        off  = (off_t)(seed % (unsigned long)nrec) * 64;
        sum += p[off];
    }
    t2 = now_ms();

    printf("64 Byte Record 를 무작위로 %d 회 읽기 (모두 Cache Hit)\n", COUNT);
    printf("pread : %8.1f ms  (%4.0f ns/회)  System Call %d 회\n", t1 - t0, (t1 - t0) * 1e6 / COUNT, COUNT);
    printf("mmap  : %8.1f ms  (%4.0f ns/회)  System Call 0 회\n", t2 - t1, (t2 - t1) * 1e6 / COUNT);
    printf("(checksum %lu)\n", sum);
    munmap(p, (size_t)st.st_size);
    close(fd);
    return EXIT_SUCCESS;
}
