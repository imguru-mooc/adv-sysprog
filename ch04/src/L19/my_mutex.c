/* my_mutex.c - futex 로 mutex 를 직접 만든다 (Ulrich Drepper, "Futexes Are Tricky" 의 방식)
 *
 *   state 0 : 잠겨 있지 않음
 *   state 1 : 잠김, 기다리는 Thread 없음
 *   state 2 : 잠김, 기다리는 Thread 가 있을 수 있음
 *
 * 경쟁이 없으면 lock 과 unlock 모두 Atomic 명령 하나로 끝나고 Kernel 에 가지 않는다.
 */
#define _GNU_SOURCE
#include <linux/futex.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

static atomic_int  state;
static atomic_long n_wait, n_wake;          /* futex System Call 횟수 */
static long        counter;
static long        loops = 200000;

static void my_lock(void)
{
    int c = 0;

    if (atomic_compare_exchange_strong(&state, &c, 1))
        return;                             /* Fast Path: 0 → 1 성공. System Call 없음 */
    if (c != 2)
        c = atomic_exchange(&state, 2);     /* "기다리는 Thread 있음" 으로 표시 */
    while (c != 0) {
        atomic_fetch_add(&n_wait, 1);
        syscall(SYS_futex, &state, FUTEX_WAIT, 2, NULL, NULL, 0);   /* state 가 2 인 동안 잔다 */
        c = atomic_exchange(&state, 2);
    }
}

static void my_unlock(void)
{
    if (atomic_fetch_sub(&state, 1) != 1) { /* 1 → 0 이면 기다리는 Thread 가 없다: 끝 */
        atomic_store(&state, 0);            /* 2 였다: 누군가 자고 있을 수 있다 */
        atomic_fetch_add(&n_wake, 1);
        syscall(SYS_futex, &state, FUTEX_WAKE, 1, NULL, NULL, 0);
    }
}

static void *worker(void *arg)
{
    long i;
    int  k;

    (void)arg;
    for (i = 0; i < loops; i++) {
        my_lock();
        for (k = 0; k < 100; k++)
            counter++;
        my_unlock();
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    pthread_t tid[64];
    int       i, n = (argc > 1) ? atoi(argv[1]) : 4;

    if (n < 1 || n > 64)
        return EXIT_FAILURE;
    for (i = 0; i < n; i++)
        if (pthread_create(&tid[i], NULL, worker, NULL) != 0)
            return EXIT_FAILURE;
    for (i = 0; i < n; i++)
        pthread_join(tid[i], NULL);
    printf("threads %d\ncounter  %ld (expected %ld)\n", n, counter, (long)n * loops * 100);
    printf("lock 호출 %ld 회 중 FUTEX_WAIT %ld 회, FUTEX_WAKE %ld 회\n",
           (long)n * loops, atomic_load(&n_wait), atomic_load(&n_wake));
    return EXIT_SUCCESS;
}
