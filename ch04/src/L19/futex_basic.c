/* futex_basic.c - futex System Call 을 직접 사용한다: "값이 아직 0 이면 재워 줘" / "자는 Thread 를 깨워 줘" */
#define _GNU_SOURCE
#include <linux/futex.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <unistd.h>

static atomic_int ready;                    /* futex 는 32bit 정수의 "주소" 를 Key 로 사용한다 */

static long futex(atomic_int *uaddr, int op, int val)
{
    return syscall(SYS_futex, uaddr, op, val, NULL, NULL, 0);
}

static void *waiter(void *arg)
{
    (void)arg;
    printf("waiter: ready 가 0 인 동안 잠듭니다\n");
    while (atomic_load(&ready) == 0) {
        /* Kernel 이 "*uaddr == 0 인가" 를 다시 확인한 뒤에만 재운다.
         * 그 사이 값이 바뀌었으면 자지 않고 EAGAIN 으로 즉시 return 한다. */
        futex(&ready, FUTEX_WAIT, 0);
    }
    printf("waiter: 깨어났습니다. ready = %d\n", atomic_load(&ready));
    return NULL;
}

int main(void)
{
    pthread_t tid;
    long      woken;

    if (pthread_create(&tid, NULL, waiter, NULL) != 0)
        return EXIT_FAILURE;
    sleep(1);
    atomic_store(&ready, 1);                /* 1. 먼저 값을 바꾸고 */
    woken = futex(&ready, FUTEX_WAKE, 1);   /* 2. 자는 Thread 를 최대 1 개 깨운다 */
    printf("main  : FUTEX_WAKE 가 깨운 Thread 수 = %ld\n", woken);
    pthread_join(tid, NULL);
    return EXIT_SUCCESS;
}
