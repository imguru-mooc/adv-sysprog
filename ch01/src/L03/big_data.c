/* big_data.c - 10MB 배열에 0 이 아닌 초기값이 있다: .data */
#include <stdio.h>

static char big[10 * 1024 * 1024] = { 1 };

int main(void)
{
    printf("big[0] = %d\n", big[0]);
    return 0;
}
