/* rowcol.c - 같은 계산, 다른 Memory 접근 순서: 행 우선 대 열 우선
 * 사용법: ./rowcol row | col
 *   perf stat -e cache-misses,cache-references ./rowcol col     (Hardware Counter: Native Linux 에서만)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 4096

static int m[N][N];                                     /* 64MB */

int main(int argc, char *argv[])
{
    struct timespec t0, t1;
    long            sum = 0;
    int             i, j, col = (argc > 1 && strcmp(argv[1], "col") == 0);

    memset(m, 1, sizeof(m));
    clock_gettime(CLOCK_MONOTONIC, &t0);
    if (!col) {
        for (i = 0; i < N; i++)
            for (j = 0; j < N; j++)
                sum += m[i][j];                         /* Memory 에 놓인 순서대로: Cache 친화적 */
    } else {
        for (j = 0; j < N; j++)
            for (i = 0; i < N; i++)
                sum += m[i][j];                         /* 16KB 씩 건너뛰며 접근: Cache Miss */
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("%s 우선: sum = %ld, %.1f ms\n", col ? "열(col)" : "행(row)", sum,
           (double)(t1.tv_sec - t0.tv_sec) * 1e3 + (double)(t1.tv_nsec - t0.tv_nsec) / 1e6);
    return EXIT_SUCCESS;
}
