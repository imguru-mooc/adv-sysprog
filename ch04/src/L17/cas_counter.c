/* cas_counter.c - Compare-And-Swap 으로 "읽기 → 계산 → 쓰기" 를 직접 구현하고 재시도 횟수를 센다 */
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

static atomic_long counter;
static atomic_long retries;
static long        loops = 5000000;

static void *worker(void *arg)
{
    long i, old, my_retries = 0;

    (void)arg;
    for (i = 0; i < loops; i++) {
        old = atomic_load(&counter);
        /* counter 가 아직 old 와 같으면 old + 1 로 바꾼다.
         * 그 사이 다른 Thread 가 바꿨다면 실패하고, old 에 현재 값이 들어온다. */
        while (!atomic_compare_exchange_weak(&counter, &old, old + 1))
            my_retries++;
    }
    atomic_fetch_add(&retries, my_retries);
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t tid[64];
    int       i, n = (argc > 1) ? atoi(argv[1]) : 2;

    if (n < 1 || n > 64)
        return EXIT_FAILURE;
    for (i = 0; i < n; i++)
        if (pthread_create(&tid[i], NULL, worker, NULL) != 0)
            return EXIT_FAILURE;
    for (i = 0; i < n; i++)
        pthread_join(tid[i], NULL);
    printf("expected %ld\nactual   %ld\nCAS 재시도 %ld 회\n",
           (long)n * loops, atomic_load(&counter), atomic_load(&retries));
    return EXIT_SUCCESS;
}
