/* vsz_rss.c - 할당(Virtual)과 실제 사용(Resident)은 다르다 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE (1024UL * 1024 * 1024)         /* 1 GB */

static void show(const char *tag)
{
    char  line[256];
    FILE *fp = fopen("/proc/self/status", "r");

    if (fp == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    printf("== %s\n", tag);
    while (fgets(line, sizeof(line), fp) != NULL)
        if (strncmp(line, "VmSize", 6) == 0 || strncmp(line, "VmRSS", 5) == 0)
            printf("   %s", line);
    fclose(fp);
}

int main(void)
{
    char  *p;
    size_t i;

    show("start");

    p = malloc(SIZE);
    if (p == NULL) {
        perror("malloc");
        return EXIT_FAILURE;
    }
    show("after malloc(1GB)          : 주소 범위만 예약");

    for (i = 0; i < SIZE / 4; i += 4096)    /* Page 마다 1 Byte 씩만 쓴다 */
        ((volatile char *)p)[i] = 1;
    show("after touching 256MB       : 건드린 만큼만 Physical Memory");

    free(p);
    show("after free");
    return EXIT_SUCCESS;
}
