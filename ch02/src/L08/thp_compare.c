/* thp_compare.c - Page 를 흩어진 순서로 방문하는 같은 접근을 4KB Page 와 2MB Huge Page(THP) 에서 비교한다
 * 사용법: ./thp_compare        (4KB Page)
 *         ./thp_compare huge   (Transparent Huge Page 요청)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>

#define PAGE   4096UL
#define SIZE   (512UL * 1024 * 1024)
#define NPAGES (SIZE / PAGE)
#define ROUNDS 16

static double now(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

static void show_huge(void)
{
    char  line[256];
    long  kb, total = 0;
    FILE *fp = fopen("/proc/self/smaps", "r");

    if (fp == NULL)
        return;
    while (fgets(line, sizeof(line), fp) != NULL)
        if (sscanf(line, "AnonHugePages: %ld kB", &kb) == 1)
            total += kb;
    fclose(fp);
    printf("AnonHugePages  : %ld kB\n", total);
}

int main(int argc, char *argv[])
{
    int            huge = (argc > 1 && strcmp(argv[1], "huge") == 0);
    unsigned long  i, r, sum = 0;
    double         t0, t1;
    volatile char *p;
    void          *raw;

    if (posix_memalign(&raw, 2UL * 1024 * 1024, SIZE) != 0) {   /* 2MB 경계에 맞춘다 */
        perror("posix_memalign");
        return EXIT_FAILURE;
    }
    if (madvise(raw, SIZE, huge ? MADV_HUGEPAGE : MADV_NOHUGEPAGE) != 0)
        perror("madvise");
    p = raw;

    for (i = 0; i < SIZE; i += PAGE)
        p[i] = 1;

    t0 = now();
    for (r = 0; r < ROUNDS; r++)
        for (i = 0; i < NPAGES; i++)                /* Page 를 흩어진 순서로 방문한다 */
            sum += p[((i * 7919UL) % NPAGES) * PAGE];
    t1 = now();

    printf("Mode           : %s\n", huge ? "2MB Huge Page (MADV_HUGEPAGE)" : "4KB Page");
    show_huge();
    printf("필요한 TLB Entry: %lu 개\n", huge ? SIZE / (2UL * 1024 * 1024) : SIZE / PAGE);
    printf("흩어진 접근     : %7.1f ms   (checksum %lu)\n", (t1 - t0) * 1e3, sum);
    free(raw);
    return EXIT_SUCCESS;
}
