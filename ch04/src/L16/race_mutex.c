/* race_mutex.c - 같은 작업을 mutex 로 보호한다. 결과는 맞지만 시간이 든다 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static long            counter;
static long            loops = 10000000;
static pthread_mutex_t lock  = PTHREAD_MUTEX_INITIALIZER;

static void *worker(void *arg)
{
    long i;

    (void)arg;
    for (i = 0; i < loops; i++) {
        pthread_mutex_lock(&lock);
        counter++;
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    struct timespec t0, t1;
    pthread_t       tid[64];
    int             i, n = (argc > 1) ? atoi(argv[1]) : 2;

    if (n < 1 || n > 64)
        return EXIT_FAILURE;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (i = 0; i < n; i++)
        if (pthread_create(&tid[i], NULL, worker, NULL) != 0)
            return EXIT_FAILURE;
    for (i = 0; i < n; i++)
        pthread_join(tid[i], NULL);
    clock_gettime(CLOCK_MONOTONIC, &t1);

    printf("expected %ld\n", (long)n * loops);
    printf("actual   %ld   %.0f ms\n", counter,
           (double)(t1.tv_sec - t0.tv_sec) * 1e3 + (double)(t1.tv_nsec - t0.tv_nsec) / 1e6);
    return EXIT_SUCCESS;
}
