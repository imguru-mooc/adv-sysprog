/* vmpte.c - Page Table 자체도 Memory 를 차지한다: VmPTE 의 증가를 관찰한다 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

#define SIZE (1024UL * 1024 * 1024)     /* 1 GB */

static void show(const char *tag)
{
    char  line[256];
    FILE *fp = fopen("/proc/self/status", "r");

    if (fp == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    printf("%-34s", tag);
    while (fgets(line, sizeof(line), fp) != NULL)
        if (strncmp(line, "VmRSS", 5) == 0 || strncmp(line, "VmPTE", 5) == 0) {
            line[strcspn(line, "\n")] = '\0';
            printf("  %s", line);
        }
    printf("\n");
    fclose(fp);
}

int main(void)
{
    size_t i;
    char  *p = mmap(NULL, SIZE, PROT_READ | PROT_WRITE,
                    MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);

    if (p == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }
    madvise(p, SIZE, MADV_NOHUGEPAGE);  /* 4KB Page 기준으로 관찰 */

    show("mmap(1GB) 직후");
    for (i = 0; i < SIZE / 2; i += 4096)
        ((volatile char *)p)[i] = 1;
    show("512MB 를 Page 마다 건드린 뒤");

    munmap(p, SIZE);
    show("munmap 뒤");
    return EXIT_SUCCESS;
}
