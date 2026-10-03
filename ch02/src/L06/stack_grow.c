/* stack_grow.c - 재귀 호출로 Stack 을 사용하면 [stack] 영역이 자동으로 커진다 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void show_stack(const char *tag)
{
    char          line[512];
    unsigned long lo, hi;
    FILE         *fp = fopen("/proc/self/maps", "r");

    if (fp == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    while (fgets(line, sizeof(line), fp) != NULL)
        if (strstr(line, "[stack]") != NULL && sscanf(line, "%lx-%lx", &lo, &hi) == 2)
            printf("%-14s [stack] %lx-%lx  %6lu KB\n", tag, lo, hi, (hi - lo) / 1024);
    fclose(fp);
}

static int dive(int depth)
{
    volatile char pad[16 * 1024];           /* 호출마다 16KB 의 지역 변수 */

    pad[0] = (char)depth;
    if (depth == 0) {
        show_stack("depth 256");
        return pad[0];
    }
    return dive(depth - 1) + pad[0];
}

int main(void)
{
    show_stack("start");
    dive(256);                              /* 약 4MB 사용 */
    show_stack("after return");
    return EXIT_SUCCESS;
}
