/* compare3.c - 같은 Counter 를 세 가지 방법으로 보호하고 시간을 비교한다
 * 사용법: ./compare3 [thread 수=2]
 */
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LOOPS 5000000L

static long            plain;
static atomic_long     atom;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
static int             mode;

static void *worker(void *arg)
{
    long i;

    (void)arg;
    for (i = 0; i < LOOPS; i++) {
        if (mode == 0) {
            pthread_mutex_lock(&mtx);
            plain++;
            pthread_mutex_unlock(&mtx);
        } else if (mode == 1) {
            atomic_fetch_add(&atom, 1);
        } else {
            long old = atomic_load(&atom);
            while (!atomic_compare_exchange_weak(&atom, &old, old + 1))
                ;
        }
    }
    return NULL;
}

static void run(int m, const char *name, int n)
{
    struct timespec t0, t1;
    pthread_t       tid[64];
    int             i;

    mode = m;
    plain = 0;
    atomic_store(&atom, 0);
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (i = 0; i < n; i++)
        pthread_create(&tid[i], NULL, worker, NULL);
    for (i = 0; i < n; i++)
        pthread_join(tid[i], NULL);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("%-18s result %9ld   %7.0f ms\n", name, m == 0 ? plain : atomic_load(&atom),
           (double)(t1.tv_sec - t0.tv_sec) * 1e3 + (double)(t1.tv_nsec - t0.tv_nsec) / 1e6);
}

int main(int argc, char *argv[])
{
    int n = (argc > 1) ? atoi(argv[1]) : 2;

    if (n < 1 || n > 64)
        return EXIT_FAILURE;
    printf("threads = %d, loops = %ld\n", n, LOOPS);
    run(0, "pthread_mutex", n);
    run(1, "atomic_fetch_add", n);
    run(2, "CAS loop", n);
    return EXIT_SUCCESS;
}
