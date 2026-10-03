/* big_bss.c - 10MB 배열이지만 초기값이 없다: .bss */
#include <stdio.h>

static char big[10 * 1024 * 1024];

int main(void)
{
    printf("big[0] = %d\n", big[0]);
    return 0;
}
