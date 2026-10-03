/* mutex_nocontend.c - 경쟁이 없는 mutex lock/unlock 100 만 번. System Call 은 몇 번일까?
 * 확인: strace -c -e trace=futex ./mutex_nocontend
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void)
{
    pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
    struct timespec t0, t1;
    long            i, n = 1000000, sum = 0;
    double          ns;

    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (i = 0; i < n; i++) {
        pthread_mutex_lock(&m);
        sum += i;
        pthread_mutex_unlock(&m);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    ns = (double)(t1.tv_sec - t0.tv_sec) * 1e9 + (double)(t1.tv_nsec - t0.tv_nsec);
    printf("lock/unlock %ld 회, 1 회당 %.1f ns  (sum=%ld)\n", n, ns / (double)n, sum);
    return EXIT_SUCCESS;
}
