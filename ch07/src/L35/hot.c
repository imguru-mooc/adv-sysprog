/* hot.c - CPU 시간을 어느 함수가 쓰는지 perf 로 찾는다
 *   checksum(): 약 70%     normalize(): 약 20%     format(): 약 10%
 */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 4096

static double data[N];

__attribute__((noinline)) static unsigned long checksum(const double *v, int n)
{
    unsigned long h = 1469598103934665603UL;
    int           i, k;

    for (k = 0; k < 7; k++)                             /* 일부러 같은 일을 7 번 반복한다 */
        for (i = 0; i < n; i++) {
            h ^= (unsigned long)(v[i] * 1000.0);
            h *= 1099511628211UL;
        }
    return h;
}

__attribute__((noinline)) static void normalize(double *v, int n)
{
    double sum = 0;
    int    i;

    for (i = 0; i < n; i++)
        sum += sqrt(fabs(v[i]) + 1.0);
    for (i = 0; i < n; i++)
        v[i] = v[i] / (sum / n) + 0.5;
}

__attribute__((noinline)) static int format(const double *v, int n, char *out, size_t cap)
{
    int i, len = 0;

    for (i = 0; i < n && (size_t)len + 16 < cap; i += 64)
        len += snprintf(out + len, cap - (size_t)len, "%.3f,", v[i]);
    return len;
}

__attribute__((noinline)) static unsigned long process_batch(int round)
{
    char          out[2048];
    unsigned long h;

    data[round % N] += 1.0;
    normalize(data, N);
    h = checksum(data, N);
    return h + (unsigned long)format(data, N, out, sizeof(out));
}

int main(int argc, char *argv[])
{
    int           rounds = argc > 1 ? atoi(argv[1]) : 20000, i;
    unsigned long acc = 0;

    for (i = 0; i < N; i++)
        data[i] = (double)i;
    for (i = 0; i < rounds; i++)
        acc += process_batch(i);
    printf("result = %lu\n", acc);
    return EXIT_SUCCESS;
}
