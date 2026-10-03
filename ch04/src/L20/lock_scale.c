/* lock_scale.c - Thread 를 늘리면 빨라지는가? 세 가지 설계를 비교한다
 *   global : 모든 Thread 가 하나의 mutex 로 하나의 Counter 를 갱신
 *   atomic : Lock 없이 atomic_fetch_add (그래도 Cache Line 은 공유된다)
 *   local  : Thread 마다 자기 Counter(Cache Line 분리), 마지막에 합산
 * 사용법: ./lock_scale [최대 thread 수=4]
 */
#include <pthread.h>
#include <stdalign.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TOTAL 40000000L                     /* 전체 작업량은 고정. Thread 수로 나눠 가진다 */

static long            g_counter;
static atomic_long     a_counter;
static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static struct { alignas(64) long v; } local[64];
static int  mode;
static long per_thread;

static void *worker(void *arg)
{
    long i, id = (long)arg;

    for (i = 0; i < per_thread; i++) {
        if (mode == 0) {
            pthread_mutex_lock(&g_lock);
            g_counter++;
            pthread_mutex_unlock(&g_lock);
        } else if (mode == 1) {
            atomic_fetch_add(&a_counter, 1);
        } else {
            local[id].v++;
        }
    }
    return NULL;
}

static double run(int m, int n)
{
    struct timespec t0, t1;
    pthread_t       tid[64];
    long            i;

    mode = m;
    per_thread = TOTAL / n;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    for (i = 0; i < n; i++)
        pthread_create(&tid[i], NULL, worker, (void *)i);
    for (i = 0; i < n; i++)
        pthread_join(tid[i], NULL);
    clock_gettime(CLOCK_MONOTONIC, &t1);
    return (double)(t1.tv_sec - t0.tv_sec) * 1e3 + (double)(t1.tv_nsec - t0.tv_nsec) / 1e6;
}

int main(int argc, char *argv[])
{
    int n, max = (argc > 1) ? atoi(argv[1]) : 4;

    if (max < 1 || max > 64)
        return EXIT_FAILURE;
    printf("전체 작업량 %ld 회 고정 (단위 ms)\n", TOTAL);
    printf("%8s %10s %10s %10s\n", "threads", "global", "atomic", "local");
    for (n = 1; n <= max; n *= 2)
        printf("%8d %10.0f %10.0f %10.0f\n", n, run(0, n), run(1, n), run(2, n));
    return EXIT_SUCCESS;
}
