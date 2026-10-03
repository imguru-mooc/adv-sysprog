/* dice.c - LD_PRELOAD 실습 대상. rand() 와 puts() 를 사용한다 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void)
{
    int i;

    srand((unsigned)time(NULL));
    puts("rolling...");
    for (i = 0; i < 5; i++)
        printf("%d ", rand() % 6 + 1);
    printf("\n");
    return 0;
}
