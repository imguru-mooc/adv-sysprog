/* zero_page.c - 읽기만 한 Anonymous Page 는 모두 하나의 "Zero Page" 를 공유한다 */
#include <string.h>
#include <sys/mman.h>
#include "faultlib.h"

#define SIZE (256UL * 1024 * 1024)
#define PAGE 4096UL

static long rss_kb(void)
{
    char  line[256];
    long  kb = -1;
    FILE *fp = fopen("/proc/self/status", "r");

    if (fp == NULL)
        return -1;
    while (fgets(line, sizeof(line), fp) != NULL)
        if (sscanf(line, "VmRSS: %ld kB", &kb) == 1)
            break;
    fclose(fp);
    return kb;
}

int main(void)
{
    long           mn0, mj, mn1, mn2;
    unsigned long  i, sum = 0;
    volatile char *p = mmap(NULL, SIZE, PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (p == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }
    madvise((void *)p, SIZE, MADV_NOHUGEPAGE);
    faults(&mn0, &mj);
    printf("%-26s RSS %7ld kB\n", "mmap(256MB) 직후", rss_kb());

    for (i = 0; i < SIZE; i += PAGE)
        sum += p[i];                        /* 읽기만 */
    faults(&mn1, &mj);
    printf("%-26s RSS %7ld kB   minor +%ld   (sum=%lu)\n", "모든 Page 읽기", rss_kb(), mn1 - mn0, sum);

    for (i = 0; i < SIZE; i += PAGE)
        p[i] = 1;                           /* 쓰기 */
    faults(&mn2, &mj);
    printf("%-26s RSS %7ld kB   minor +%ld\n", "모든 Page 쓰기", rss_kb(), mn2 - mn1);

    munmap((void *)p, SIZE);
    return EXIT_SUCCESS;
}
