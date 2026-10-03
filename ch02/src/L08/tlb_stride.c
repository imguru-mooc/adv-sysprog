/* tlb_stride.c - 같은 횟수의 Memory 접근이라도 "몇 개의 Page 를 오가는가"에 따라 속도가 달라진다
 *
 *   dense  : 연속된 주소를 접근한다. 접근 4096 번당 새 Page 1 개 (TLB Hit 대부분)
 *   sparse : Page 마다 1 Byte 씩 접근한다. 접근할 때마다 다른 Page (TLB Miss 다발)
 *
 * 두 경우 모두 접근 횟수는 같고, 미리 모든 Page 를 건드려 Page Fault 의 영향을 없앤다.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>

#define PAGE   4096UL
#define NPAGES (256UL * 1024)               /* 256K Pages = 1 GB 의 주소 범위 */
#define ROUNDS 8

static double now(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

int main(void)
{
    unsigned long  i, r, sum = 0;
    double         t0, t1, t2;
    volatile char *p = mmap(NULL, NPAGES * PAGE, PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);

    if (p == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }
    madvise((void *)p, NPAGES * PAGE, MADV_NOHUGEPAGE);

    for (i = 0; i < NPAGES; i++)            /* 준비: 모든 Page 를 미리 Physical Memory 에 올린다 */
        p[i * PAGE] = 1;

    t0 = now();
    for (r = 0; r < ROUNDS; r++)
        for (i = 0; i < NPAGES; i++)        /* dense: 앞쪽 64 Pages 안에서만 접근 */
            sum += p[i];
    t1 = now();
    for (r = 0; r < ROUNDS; r++)
        for (i = 0; i < NPAGES; i++)        /* sparse: 접근마다 다른 Page */
            sum += p[i * PAGE];
    t2 = now();

    printf("접근 횟수      : %lu 회씩\n", ROUNDS * NPAGES);
    printf("dense  (64 Pages)     : %7.1f ms\n", (t1 - t0) * 1e3);
    printf("sparse (262144 Pages) : %7.1f ms   (%.1f 배)\n", (t2 - t1) * 1e3, (t2 - t1) / (t1 - t0));
    printf("checksum %lu\n", sum);
    munmap((void *)p, NPAGES * PAGE);
    return EXIT_SUCCESS;
}
