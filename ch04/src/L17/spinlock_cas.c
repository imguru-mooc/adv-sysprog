/* spinlock_cas.c - atomic_flag(test-and-set) 하나로 Spin Lock 을 만든다 */
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

static atomic_flag lock_flag = ATOMIC_FLAG_INIT;
static long        counter;                 /* 일반 변수. Lock 이 보호한다 */
static long        loops = 5000000;

static void spin_lock(void)
{
    while (atomic_flag_test_and_set(&lock_flag))    /* 이미 1 이었으면 다시 시도 */
        __builtin_ia32_pause();                     /* PAUSE: Spin 중임을 CPU 에 알린다 */
}

static void spin_unlock(void)
{
    atomic_flag_clear(&lock_flag);
}

static void *worker(void *arg)
{
    long i;

    (void)arg;
    for (i = 0; i < loops; i++) {
        spin_lock();
        counter++;
        spin_unlock();
    }
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
    printf("expected %ld\nactual   %ld\n", (long)n * loops, counter);
    return EXIT_SUCCESS;
}
