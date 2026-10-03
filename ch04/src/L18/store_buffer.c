/* store_buffer.c - 두 CPU 가 각각 "쓰고 나서 상대 변수를 읽는다"
 *
 *      Thread 1            Thread 2
 *      x = 1;              y = 1;
 *      r1 = y;             r2 = x;
 *
 * 어떤 순서로 끼워 넣어도 (r1, r2) = (0, 0) 은 나올 수 없을 것 같다.
 * 그러나 x86 에서도 나온다: 쓰기가 Store Buffer 에 머무는 동안 상대의 읽기가 먼저 실행되기 때문.
 * -DUSE_ATOMIC: atomic_store(seq_cst) 는 xchg 를 사용해 Store 직후 Full Barrier 가 된다.
 * CPU 가 2 개 이상이어야 관찰된다.
 */
#define _GNU_SOURCE
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>

#define ROUNDS 100000

#ifdef USE_ATOMIC
static atomic_int x, y;
#define WRITE(v)  atomic_store(&(v), 1)
#define READ(v)   atomic_load(&(v))
#define RESET(v)  atomic_store(&(v), 0)
#else
static volatile int x, y;                   /* volatile: Compiler 최적화는 막지만 CPU 는 막지 못한다 */
#define WRITE(v)  ((v) = 1)
#define READ(v)   (v)
#define RESET(v)  ((v) = 0)
#endif

static int               r1, r2;
static pthread_barrier_t go, done;

static void pin(int cpu)
{
    cpu_set_t set;

    CPU_ZERO(&set);
    CPU_SET(cpu, &set);
    pthread_setaffinity_np(pthread_self(), sizeof(set), &set);     /* 실패해도 계속 진행 */
}

/* 두 Thread 의 출발 시점을 조금씩 어긋나게 해서 다양한 겹침을 만든다 */
static void jitter(unsigned *seed)
{
    volatile int k;

    *seed = *seed * 1103515245u + 12345u;
    for (k = (int)((*seed >> 16) & 7); k > 0; k--)
        ;
}

static void *t1(void *arg)
{
    int i;

    unsigned seed = 1;

    (void)arg;
    pin(0);
    for (i = 0; i < ROUNDS; i++) {
        pthread_barrier_wait(&go);
        jitter(&seed);
        WRITE(x);
        r1 = READ(y);
        pthread_barrier_wait(&done);
    }
    return NULL;
}

static void *t2(void *arg)
{
    int i;

    unsigned seed = 2;

    (void)arg;
    pin(1);
    for (i = 0; i < ROUNDS; i++) {
        pthread_barrier_wait(&go);
        jitter(&seed);
        WRITE(y);
        r2 = READ(x);
        pthread_barrier_wait(&done);
    }
    return NULL;
}

int main(void)
{
    pthread_t a, b;
    long      c00 = 0, c01 = 0, c10 = 0, c11 = 0;
    int       i;

    pthread_barrier_init(&go, NULL, 3);
    pthread_barrier_init(&done, NULL, 3);
    pthread_create(&a, NULL, t1, NULL);
    pthread_create(&b, NULL, t2, NULL);

    for (i = 0; i < ROUNDS; i++) {
        RESET(x);
        RESET(y);
        pthread_barrier_wait(&go);
        pthread_barrier_wait(&done);
        if (r1 == 0 && r2 == 0) c00++;
        else if (r1 == 0)       c01++;
        else if (r2 == 0)       c10++;
        else                    c11++;
    }
    pthread_join(a, NULL);
    pthread_join(b, NULL);

    printf("rounds %d\n", ROUNDS);
    printf("(r1,r2) = (0,1) %8ld\n(r1,r2) = (1,0) %8ld\n(r1,r2) = (1,1) %8ld\n", c01, c10, c11);
    printf("(r1,r2) = (0,0) %8ld   <- 순차적으로는 불가능한 결과\n", c00);
    return EXIT_SUCCESS;
}
