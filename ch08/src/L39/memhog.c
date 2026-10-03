/* memhog.c - 10MB 씩 할당하고 실제로 써서 RSS 를 늘린다. 한도에 닿으면 어떻게 되는지 본다
 * 사용법: ./memhog [최대 MB=200]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int max_mb = argc > 1 ? atoi(argv[1]) : 200, mb;

    for (mb = 10; mb <= max_mb; mb += 10) {
        char *p = malloc(10u << 20);

        if (p == NULL) {
            printf("malloc 실패 (%d MB)\n", mb);
            return EXIT_FAILURE;
        }
        memset(p, 1, 10u << 20);                        /* 써야 Physical Memory 가 할당된다 (L09) */
        printf("%4d MB 사용 중\n", mb);
        fflush(stdout);
        usleep(100 * 1000);
    }
    printf("끝까지 도달: %d MB\n", max_mb);
    return EXIT_SUCCESS;
}
